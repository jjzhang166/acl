#include "stdafx.h"
#include <poll.h>
#include <netdb.h>
#define __USE_GNU
#include <dlfcn.h>
#include "event.h"
#include "fiber_schedule.h"
#include "fiber.h"

typedef int (*accept_fn)(int, struct sockaddr *, socklen_t *);
typedef int (*connect_fn)(int, const struct sockaddr *, socklen_t);

static accept_fn   __sys_accept   = NULL;
static connect_fn  __sys_connect  = NULL;

void fiber_net_hook(void)
{
	static int __called = 0;

	if (__called)
		return;

	__called++;

	__sys_accept   = (accept_fn) dlsym(RTLD_NEXT, "accept");
	__sys_connect  = (connect_fn) dlsym(RTLD_NEXT, "connect");
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int   clifd;

	acl_non_blocking(sockfd, ACL_NON_BLOCKING);

	while (1) {
		clifd = __sys_accept(sockfd, addr, addrlen);

		if (clifd >= 0) {
			acl_non_blocking(clifd, ACL_NON_BLOCKING);
			acl_tcp_nodelay(clifd, 1);
			return clifd;
		}

#if EAGAIN == EWOULDBLOCK
		if (errno != EAGAIN)
#else
		if (errno != EAGAIN && errno != EWOULDBLOCK)
#endif
			return -1;

		fiber_wait_read(sockfd);
	}
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	acl_non_blocking(sockfd, ACL_NON_BLOCKING);

	while (1) {
		int ret = __sys_connect(sockfd, addr, addrlen);
		if (ret >= 0) {
			acl_tcp_nodelay(sockfd, 1);
			return ret;
		}

#if EAGAIN == EWOULDBLOCK
		if (errno != EAGAIN)
#else
		if (errno != EAGAIN && errno != EWOULDBLOCK)
#endif
			return -1;

		fiber_wait_write(sockfd);
	}
}

static void poll_callback(EVENT *ev, POLL_EVENTS *pe)
{
	int i;

	for (i = 0; i < pe->nfds; i++) {
		if (pe->fds[i].events & POLLIN)
			event_del(ev, pe->fds[i].fd, EVENT_READABLE);
		if (pe->fds[i].events & POLLOUT)
			event_del(ev, pe->fds[i].fd, EVENT_WRITABLE);

		fiber_io_dec();
	}

	fiber_ready(pe->curr);
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	POLL_EVENTS pe;
	EVENT *event;

	fiber_io_check();

	event = fiber_io_event();

	pe.fds    = fds;
	pe.nfds   = nfds;
	pe.nrefer = 0;
	pe.curr   = fiber_running();
	pe.proc   = poll_callback;

	event_poll(event, &pe, timeout);

	fiber_io_inc();
	fiber_switch();

	return pe.nready;
}

int select(int nfds, fd_set *readfds, fd_set *writefds,
	fd_set *exceptfds, struct timeval *timeout)
{
	struct pollfd *fds = (struct pollfd *) acl_mycalloc(nfds + 1,
			sizeof(struct pollfd));
	int fd, timo;

	for (fd = 0; fd < nfds; fd++) {
		if (FD_ISSET(fd, readfds)) {
			fds[fd].fd = fd;
			fds[fd].events |= POLLIN;
		}

		if (FD_ISSET(fd, writefds)) {
			fds[fd].fd = fd;
			fds[fd].events |= POLLOUT;
		}

		if (FD_ISSET(fd, exceptfds)) {
			fds[fd].fd = fd;
			fds[fd].events |= POLLERR | POLLHUP;
		}
	}

	if (timeout != NULL)
		timo = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
	else
		timo = -1;

	nfds = poll(fds, nfds, timo);

	acl_myfree(fds);

	return nfds;
}

struct hostent *gethostbyname(const char *name)
{
	static __thread struct hostent ret, *result;
#define BUF_LEN	4096
	static __thread char buf[BUF_LEN];

	return gethostbyname_r(name, &ret, buf, BUF_LEN, &result, &h_errno)
		== 0 ? result : NULL;
}

int gethostbyname_r(const char *name, struct hostent *ret,
	char *buf, size_t buflen, struct hostent **result, int *h_errnop)
{
	const char *dns_ip = "8.8.8.8";
	int dns_port = 53;
	ACL_RES *ns = acl_res_new(dns_ip, dns_port);
	ACL_DNS_DB *res = NULL;
	size_t n = 0, len, i = 0;
	ACL_ITER iter;

#define	RETURN(x) do { \
	if (res) \
		acl_netdb_free(res); \
	if (ns) \
		acl_res_free(ns); \
	return (x); \
} while (0)

	memset(ret, 0, sizeof(struct hostent));
	memset(buf, 0, buflen);

	if (ns == NULL) {
		acl_msg_error("%s(%d), %s: acl_res_new NULL, name: %s,"
			" dns_ip: %s, dns_port: %d", __FILE__, __LINE__,
			__FUNCTION__, name, dns_ip, dns_port);
		RETURN (-1);
	}

	res = acl_res_lookup(ns, name);
	if (res == NULL) {
		acl_msg_error("%s(%d), %s: acl_res_lookup NULL, name: %s,"
			" dns_ip: %s, dns_port: %d", __FILE__, __LINE__,
			__FUNCTION__, name, dns_ip, dns_port);
		if (h_errnop)
			*h_errnop = HOST_NOT_FOUND;
		RETURN (-1);
	}

	len = strlen(name);
	n += len;
	if (n >= buflen) {
		acl_msg_error("%s(%d), %s: n(%d) > buflen(%d)", __FILE__,
			__LINE__, __FUNCTION__, (int) n, (int) buflen);
		if (h_errnop)
			*h_errnop = ERANGE;
		RETURN (-1);
	}
	memcpy(buf, name, len);
	buf[len] = 0;
	ret->h_name = buf;
	buf += len + 1;

#define MAX_COUNT	64
	len = 8 * MAX_COUNT;
	n += len;
	if (n >= buflen) {
		acl_msg_error("%s(%d), %s: n(%d) > buflen(%d)", __FILE__,
			__LINE__, __FUNCTION__, (int) n, (int) buflen);
		if (h_errnop)
			*h_errnop = ERANGE;
		RETURN (-1);
	}
	ret->h_addr_list = (char**) buf;
	buf += len;

	acl_foreach(iter, res) {
		ACL_HOSTNAME *h = (ACL_HOSTNAME*) iter.data;

		len = strlen(h->ip);
		n += len;
		memcpy(buf, h->ip, len);
		buf[len] = 0;

		if (i >= MAX_COUNT)
			break;
		ret->h_addr_list[i++] = buf;
		buf += len + 1;
		ret->h_length += len;
	}

	if (i == 0) {
		acl_msg_error("%s(%d), %s: i == 0",
			__FILE__, __LINE__, __FUNCTION__);
		if (h_errnop)
			*h_errnop = ERANGE;
		RETURN (-1);
	}

	*result = ret;

	RETURN (0);
}
