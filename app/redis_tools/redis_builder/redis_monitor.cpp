#include "stdafx.h"
#include "redis_util.h"
#include "redis_monitor.h"

redis_monitor::redis_monitor(const char* addr, int conn_timeout,
	int rw_timeout, const char* passwd)
{
	addr_ = addr;
	conn_timeout_ = conn_timeout;
	rw_timeout_ = rw_timeout;
	if (passwd && *passwd)
		passwd_ = passwd;
}

redis_monitor::~redis_monitor(void)
{
}

void redis_monitor::status(void)
{
	acl::redis_client client(addr_, conn_timeout_, rw_timeout_);
	acl::redis redis(&client);
	const std::map<acl::string, acl::redis_node*>* masters =
		redis_util::get_masters(redis);

	if (masters == NULL)
	{
		printf("masters NULL\r\n");
		return;
	}

	std::vector<acl::redis_client*> conns;
	for (std::map<acl::string, acl::redis_node*>::const_iterator
		cit = masters->begin(); cit != masters->end(); ++cit)
	{
		const char* addr = cit->second->get_addr();
		if (addr == NULL || *addr == 0)
		{
			printf("addr NULL, skip it\r\n");
			continue;
		}

		acl::redis_client* conn = new acl::redis_client(
			addr, conn_timeout_, rw_timeout_);
		conns.push_back(conn);
	}

	while (true)
	{
		show_status(conns);
		sleep(1);
	}

	for (std::vector<acl::redis_client*>::iterator it = conns.begin();
		it != conns.end(); ++it)
	{
		delete *it;
	}
}

int redis_monitor::check(const std::map<acl::string, acl::string>& info,
	const char* name, std::vector<int>& res)
{
	std::map<acl::string, acl::string>::const_iterator cit
		= info.find(name);
	if (cit == info.end())
	{
		printf("no %s\r\n", name);
		res.push_back(0);
		return 0;
	}
	else
	{
		int n = atoi(cit->second.c_str());
		res.push_back(n);
		return n;
	}
}

long long redis_monitor::check(const std::map<acl::string, acl::string>& info,
	const char* name, std::vector<long long>& res)
{
	std::map<acl::string, acl::string>::const_iterator cit
		= info.find(name);
	if (cit == info.end())
	{
		printf("no %s\r\n", name);
		res.push_back(0);
		return 0;
	}
	else
	{
		long long n = acl_atoll(cit->second.c_str());
		res.push_back(n);
		return n;
	}
}

int redis_monitor::check_keys(const char* name, const char* value)
{
	if (strncmp(name, "db", 2) != 0 || *value == 0)
		return 0;

	name += 2;
	if (*name == 0 || !acl_alldig(name))
		return 0;

	acl::string buf(value);
	buf.trim_space();

	int keys = 0;
	std::vector<acl::string>& tokens = buf.split2(";,");
	for (std::vector<acl::string>::iterator it = tokens.begin();
		it != tokens.end(); ++it)
	{
		std::vector<acl::string>& tokens2 = (*it).split2("=");
		if (tokens2.size() != 2)
			continue;
		if (tokens2[0] == "keys" && acl_alldig(tokens2[1]))
			keys = atoi(tokens2[1].c_str());
	}

	return keys;
}

int redis_monitor::check_keys(const std::map<acl::string, acl::string>& info,
	std::vector<int>& keys)
{
	int count = 0;

	for (std::map<acl::string, acl::string>::const_iterator cit
		= info.begin(); cit != info.end(); ++cit)
	{
		int n = check_keys(cit->first, cit->second);
		if (n > 0)
			count += n;
	}

	keys.push_back(count);
	return count;
}

void redis_monitor::show_status(std::vector<acl::redis_client*>& conns)
{
	std::vector<acl::string> addrs;
	std::vector<int> tpses;
	std::vector<int> clients;
	std::vector<int> keys;
	std::vector<long long> memory;

	for (std::vector<acl::redis_client*>::iterator it = conns.begin();
		it != conns.end(); ++it)
	{
		acl::redis cmd(*it);
		std::map<acl::string, acl::string> info;
		if (cmd.info(info) < 0)
		{
			printf("cmd info error: %s, addr: %s\r\n",
				cmd.result_error(), (*it)->get_addr());
			continue;
		}

		addrs.push_back((*it)->get_addr());
		check(info, "instantaneous_ops_per_sec", tpses);
		check(info, "connected_clients", clients);
		check_keys(info, keys);
		check(info, "used_memory", memory);
	}

	int  all_tps = 0, all_client = 0, all_keys = 0;
	size_t size = addrs.size();

#define KB	1024
#define MB	(1024 * 1024)
#define	GB	(1024 * 1024 * 1024)

	const char* units;
	double msize;
	long long total_msize = 0;

	for (size_t i = 0; i < size; i++)
	{
		if ((msize = ((double) memory[i]) / GB) >= 1)
			units = "G";
		else if ((msize = ((double) memory[i]) / MB) >= 1)
			units = "M";
		else if ((msize = ((double) memory[i]) / KB) >= 1)
			units = "K";
		else
		{
			msize = (double) memory[i];
			units = "B";
		}

#ifdef ACL_UNIX
		printf("\033[1;32;40mredis:\033[0m"
			" \033[1;36;40m%s\033[0m,"
			" \033[1;33;40mtps:\033[0m"
			" \033[1;36;40m%d\033[0m,"
			" \033[1;33;40mclients:\033[0m"
			" \033[1;36;40m%d\033[0m,"
			" \033[1;33;40mkeys:\033[0m"
			" \033[1;36;40m%d\033[0m,"
			" \033[1;33;40mmemory:\033[0m"
			" \033[1;36;40m%.2f %s\033[0m\r\n",
			addrs[i].c_str(), tpses[i], clients[i], keys[i],
			msize, units);
#else
		printf("redis: %s, tps: %d, client: %d, keys: %d,"
			" memory: %.2f %s\r\n", addrs[i].c_str(), tpses[i],
			clients[i], keys[i], msize, units);
#endif
		if (tpses[i] > 0)
			all_tps += tpses[i];
		if (clients[i] > 0)
			all_client += clients[i];
		if (keys[i] > 0)
			all_keys += keys[i];
		if (memory[i] > 0)
			total_msize += memory[i];
	}

	if ((msize = ((double) total_msize) / GB) >= 1)
		units = "G";
	else if ((msize = ((double) total_msize) / MB) >= 1)
		units = "M";
	else if ((msize = ((double) total_msize) / KB) >= 1)
		units = "K";
	else
	{
		msize = (double) total_msize;
		units = "B";
	}


#ifdef ACL_UNIX
	printf("\033[1;34;40mtotal masters:\033[0m"
		" \033[1;36;40m%d\033[0m,"
		" \033[1;34;40mtotal tps:\033[0m"
		" \033[1;36;40m%d\033[0m,"
		" \033[1;34;40mtotal client:\033[0m"
		" \033[1;36;40m%d\033[0m,"
		" \033[1;34;40mtotal keys:\033[0m"
		" \033[1;36;40m%d\033[0m,"
		" \033[1;34;40mtotal memory:\033[0m"
		" \033[1;36;40m%.2f %s\033[0m\r\n",
		(int) size, all_tps, all_client, all_keys, msize, units);
#else
	printf("total masters: %d, total tps: %d, total clients: %d,"
		" total keys: %d, total memory: %.2f %s\r\n",
		(int) size, all_tps, all_client, all_keys, msize, units);
#endif
}
