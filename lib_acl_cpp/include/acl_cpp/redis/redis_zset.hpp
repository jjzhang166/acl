#pragma once
#include "acl_cpp/acl_cpp_define.hpp"
#include <vector>
#include <map>
#include <utility>
#include "acl_cpp/redis/redis_command.hpp"

namespace acl
{

class redis_client;

class ACL_CPP_API redis_zset : public redis_command
{
public:
	redis_zset(redis_client* conn = NULL);
	~redis_zset();

	/////////////////////////////////////////////////////////////////////

	/**
	 * 添加对应 key 的有序集
	 * @param key {const char*} 有序集键值
	 * @param members "分值-成员" 对数组
	 * @return {int} 新成功添加的 "分值-成员" 对的数量
	 *  0：表示一个也未添加，可能因为该成员已经存在于有序集中
	 * -1：表示出错或 key 对象非有序集对象
	 * >0：新添加的成员数量
	 */
	int zadd(const char* key, std::map<string, double>& members);
	int zadd(const char* key,
		std::vector<std::pair<string, double> >&members);
	int zadd(const char* key,
		std::vector<std::pair<const char*, double> >&members);
	int zadd(const char* key, std::vector<string>& members,
		std::vector<double>& scores);
	int zadd(const char* key, std::vector<const char*>& members,
		std::vector<double>& scores);
	int zadd(const char* key, const char* members[], double scores[],
		size_t size);
	int zadd(const char* key, const char* members[], size_t members_len[],
		double scores[], size_t size);

	/**
	 * 获得相应键的有序集的成员数量
	 * @param key {const char*} 有序集键值
	 * @return {int} 一个键的有序集的成员数量
	 *  0：该键不存在
	 *  -1：出错或该键的数据对象不是有效的有序集对象
	 *  > 0：当前键值对应的数据对象中的成员个数
	 */
	int zcard(const char* key);

	/**
	 * 获得 key 的有序集中指定分值区间的成员个数
	 * @param key {const char*} 有序集键值
	 * @param min {double} 最小分值
	 * @param max {double} 最大分值
	 * @return {int} 符合条件的成员个数
	 *  0：该键对应的有序集不存在或该 KEY 有序集的对应分值区间成员为空
	 *  -1: 出错或该键的数据对象不是有效的有序集对象
	 */
	int zcount(const char* key, double min, double max);

	/**
	 * 将 key 的有序集中的某个成员的分值加上增量 inc
	 * @param key {const char*} 有序集键值
	 * @param inc {double} 增量值
	 * @param member{const char*} 有序集中成员名
	 * @param result {double*} 非空时存储结果值
	 * @return {bool} 操作是否成功
	 */
	bool zincrby(const char* key, double inc, const char* member,
		double* result = NULL);
	bool zincrby(const char* key, double inc, const char* member,
		size_t len, double* result = NULL);

	/**
	 * 从 key 的有序集中获得指定位置区间的成员名列表，成员按分值递增方式排序
	 * @param key {const char*} 有序集键值
	 * @param start {int} 起始下标位置
	 * @param stop {int} 结束下标位置（结果集同时含该位置）
	 * @param result {std::vector<string>&} 存储结果集
	 * @return {int} 结果集中成员的数量
	 *  0: 表示结果集为空或 key 不存在
	 * -1: 表示出错或 key 对象非有序集对象
	 * >0: 结果集的数量
	 *  注：对于下标位置，0 表示第一个成员，1 表示第二个成员；-1 表示最后一个成员，
	 *     -2 表示倒数第二个成员，以此类推
	 */
	int zrange(const char* key, int start, int stop,
		std::vector<string>& result);

	/**
	 * 从 key 的有序集中获得指定位置区间的成员名及分值列表，成员按分值递增方式排序
	 * @param key {const char*} 有序集键值
	 * @param start {int} 起始下标位置
	 * @param stop {int} 结束下标位置（结果集同时含该位置）
	 * @param result 存储 "成员名-分值对"结果集
	 * @return {int} 结果集中成员的数量
	 *  0: 表示结果集为空或 key 不存在
	 * -1: 表示出错或 key 对象非有序集对象
	 * >0: 结果集的数量
	 *  注：对于下标位置，0 表示第一个成员，1 表示第二个成员；-1 表示最后一个成员，
	 *     -2 表示倒数第二个成员，以此类推
	 */
	int zrange_with_scores(const char* key, int start, int stop,
		std::vector<std::pair<string, double> >& out);

	/**
	 * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员。有序集成员按 score 值递增(从小到大)次序排列
	 * @param key {const char*} 有序集键值
	 * @param min {double} 最小分值
	 * @param max {double} 最大分值
	 * @param out 存储“分值-成员名”对的结果集
	 * @param offset {const int*} 非空时表示结果集的起始下标
	 * @param count {const int*} 非空时表示截取的结果集中成员个数
	 * @return {int} 结果集中成员的数量
	 *  0: 表示结果集为空或 key 不存在
	 * -1: 表示出错或 key 对象非有序集对象
	 * >0: 结果集的数量
	 *  注：offset 和 count 必须同时为非空指针时才有效
	 */
	int zrangebyscore(const char* key, double min, double max,
		std::vector<string>& out, const int* offset = NULL,
		const int* count = NULL);

	/**
	 * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员。有序集成员按 score 值递增(从小到大)次序排列
	 * @param key {const char*} 有序集键值
	 * @param min {const char*} 以字符串表示最小分值
	 * @param max {const char*} 以字符串表示最大分值
	 * @param out 存储“分值-成员名”对的结果集
	 * @param offset {const int*} 非空时表示结果集的起始下标
	 * @param count {const int*} 非空时表示截取的结果集中成员个数
	 * @return {int} 结果集中成员的数量
	 *  0: 表示结果集为空或 key 不存在
	 * -1: 表示出错或 key 对象非有序集对象
	 * >0: 结果集的数量
	 *  注：
	 * 1）offset 和 count 必须同时为非空指针时才有效
	 * 2）min 和 max 可以是 -inf 和 +inf 来表示无限区间
	 * 3）默认情况下，区间的取值使用闭区间 (小于等于或大于等于)，也可以通过给参数前
	 *   增加 ( 符号来使用可选的开区间 (小于或大于)，如：
	 * 3.1）"ZRANGEBYSCORE zset (1 5" 返回所有符合条件 1 < score <= 5 的成员
	 * 3.2）"ZRANGEBYSCORE zset (5 (10" 返回所有符合条件 5 < score < 10 的成员
	 */
	int zrangebyscore(const char* key, const char* min, const char* max,
		std::vector<string>& out, const int* offset = NULL,
		const int* count = NULL);

	/**
	 * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员及分值。有序集成员按 score 值递增(从小到大)次序排列；分值(min/max)使用
	 * 浮点数表示
	 */
	int zrangebyscore_with_scores(const char* key, double min, double max,
		std::vector<std::pair<string, double> >& out,
		const int* offset = NULL, const int* count = NULL);

	/**
	 * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员及分值。有序集成员按 score 值递增(从小到大)次序排列；分值(min/max)使用
	 * 字符串表示
	 */
	int zrangebyscore_with_scores(const char* key, const char* min,
		const char* max, std::vector<std::pair<string, double> >& out,
		const int* offset = NULL, const int* count = NULL);

	/**
	 * 返回有序集 key 中成员 member 的排名(下标从 0 开始）；其中有序集成员按 score
	 * 值递增(从小到大)顺序排列
	 * @param key {const char*} 有序集键值
	 * @param member {const char*} 成员名
	 * @param len {size_t} member 的长度
	 * @return {int} 下标位置值，-1 -- 出错，或 key 非有序集对象，或成员名不存在
	 */
	int zrank(const char* key, const char* member, size_t len);
	int zrank(const char* key, const char* member);

	/**
	 * 从有序集中删除某个成员
	 * @param key {const char*} 有序集键值
	 * @param first_member {const char*} 要删除的成员列表的第一个
	 * @return {int} 成功删除的成员的数量，-1 表示出错或该 key 非有序集对象，
	 *  0 表示该有序集不存在或成员不存在，> 0 表示成功删除的成员数量
	 */
	int zrem(const char* key, const char* first_member, ...);
	int zrem(const char* key, const std::vector<string>& members);
	int zrem(const char* key, const std::vector<const char*>& members);
	int zrem(const char* key, const char* members[], const size_t lens[],
		size_t argc);

	/**
	 * 移除有序集 key 中，指定排名(rank)区间内的所有成员；
	 * 区间分别以下标参数 start 和 stop 指出，包含 start 和 stop 在内；
	 * 下标参数 start 和 stop 都以 0 为底，也就是说，以 0 表示有序集第一个成员，
	 * 以 1 表示有序集第二个成员，以此类推；
	 * 也可以使用负数下标，以 -1 表示最后一个成员， -2 表示倒数第二个成员，以此类推
	 * @param key {const char*} 有序集键值
	 * @param start {int} 起始下标位置（从 0 开始）
	 * @param stop {int} 结束下标位置
	 * @return {int} 被移除的成员数量
	 *  0：表示 key 不存在或移除的区间不存在
	 * -1：表示出错或 key 不是有序集合对象键值
	 */
	int zremrangebyrank(const char* key, int start, int stop);

	/**
	 * 移除有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员；自版本2.1.6开始，score 值等于 min 或 max 的成员也可以不包括在内，
	 * 详情请参见 ZRANGEBYSCORE 命令
	 * @param key {const char*} 有序集键值
	 * @param min {double} 最小分值
	 * @param max {double} 最大分值
	 * @return {int} 成功删除的成员的数量，-1 表示出错或该 key 非有序集对象，
	 *  0 表示该有序集不存在或成员不存在，> 0 表示成功删除的成员数量
	 */
	int zremrangebyscore(const char* key, double min, double max);

	/**
	 * 移除有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员；自版本2.1.6开始，score 值等于 min 或 max 的成员也可以不包括在内，
	 * 详情请参见 ZRANGEBYSCORE 命令
	 * @param key {const char*} 有序集键值
	 * @param min {const char*} 字符串形式的最小分值，意义参见：zrangebyscore 注释
	 * @param max {const char*} 字符串形式的最大分值
	 * @return {int} 成功删除的成员的数量，-1 表示出错或该 key 非有序集对象，
	 *  0 表示该有序集不存在或成员不存在，> 0 表示成功删除的成员数量
	 */
	int zremrangebyscore(const char* key, const char* min, const char* max);

	/**
	 * 从 key 的有序集中获得指定位置区间的成员名列表，成员按分值递减方式排序
	 * @param key {const char*} 有序集键值
	 * @param start {int} 起始下标位置
	 * @param stop {int} 结束下标位置（结果集同时含该位置）
	 * @param result {std::vector<string>&} 存储结果集
	 *  注：对于下标位置，0 表示第一个成员，1 表示第二个成员；-1 表示最后一个成员，
	 *     -2 表示倒数第二个成员，以此类推
	 */
	int zrevrange(const char* key, int start, int stop,
		std::vector<string>& result);

	/**
	 * 从 key 的有序集中获得指定位置区间的成员名及分值列表，成员按分值递减方式排序
	 * @param key {const char*} 有序集键值
	 * @param start {int} 起始下标位置
	 * @param stop {int} 结束下标位置（结果集同时含该位置）
	 * @param result 存储 "成员名-分值对"结果集
	 *  注：对于下标位置，0 表示第一个成员，1 表示第二个成员；-1 表示最后一个成员，
	 *     -2 表示倒数第二个成员，以此类推
	 */
	int zrevrange_with_scores(const char* key, int start, int stop,
		std::vector<std::pair<string, double> >& out);

	/**
	 * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员。有序集成员按 score 值递8减(从小到大)次序排列
	 * @param key {const char*} 有序集键值
	 * @param min {const char*} 以字符串表示最小分值
	 * @param max {const char*} 以字符串表示最大分值
	 * @param out 存储“分值-成员名”对的结果集
	 * @param offset {const int*} 非空时表示结果集的起始下标
	 * @param count {const int*} 非空时表示截取的结果集中成员个数
	 * @return {int} 结果集中成员的数量
	 *  0: 表示结果集为空或 key 不存在
	 * -1: 表示出错或 key 对象非有序集对象
	 * >0: 结果集的数量
	 *  注：
	 * 1）offset 和 count 必须同时为非空指针时才有效
	 * 2）min 和 max 可以是 -inf 和 +inf 来表示无限区间
	 * 3）默认情况下，区间的取值使用闭区间 (小于等于或大于等于)，也可以通过给参数前
	 *   增加 ( 符号来使用可选的开区间 (小于或大于)，如：
	 * 3.1）"ZRANGEBYSCORE zset (1 5" 返回所有符合条件 1 < score <= 5 的成员
	 * 3.2）"ZRANGEBYSCORE zset (5 (10" 返回所有符合条件 5 < score < 10 的成员
	 */
	int zrevrangebyscore(const char* key, const char* min, const char* max,
		std::vector<string>& out, const int* offset = NULL,
		const int* count = NULL);

	/**
	 * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
	 * 的成员及分值。有序集成员按 score 值递减(从小到大)次序排列；分值(min/max)使用
	 * 浮点数表示
	 */
	int zrevrangebyscore_with_scores(const char* key, double min,
		double max, std::vector<std::pair<string, double> >& out,
		const int* offset = NULL, const int* count = NULL);
	int zrevrangebyscore_with_scores(const char* key, const char* min,
		const char* max, std::vector<std::pair<string, double> >& out,
		const int* offset = NULL, const int* count = NULL);

	/**
	 * 返回有序集 key 中成员 member 的排名(下标从 0 开始)；其中有序集成员按 score
	 * 值递减(从大到小)排序
	 * @param key {const char*} 有序集键值
	 * @param member {const char*} 成员名
	 * @param len {size_t} member 的长度
	 * @return {int} 下标位置值，-1 -- 出错，或 key 非有序集对象，或成员名不存在
	 */
	int zrevrank(const char* key, const char* member, size_t len);
	int zrevrank(const char* key, const char* member);

	/**
	 * 获得有序集 key 中，成员 member 的 score 值
	 * @param key {const char*} 有序集键值
	 * @param member {const char*} 成员名
	 * @param len {size_t} member 的长度
	 * @param result {double&} 存储分值结果
	 * @return {bool} 当不存在或出错时返回 false，否则返回 true
	 */
	bool zscore(const char* key, const char* member, size_t len,
		double& result);
	bool zscore(const char* key, const char* member, double& result);

	/**
	 * 计算给定的一个或多个有序集的并集，其中给定 key 的数量必须以 numkeys 参数指定，
	 * 并将该并集(结果集)储存到目标有序集; 默认情况下，结果集中某个成员的 score
	 * 值是所有给定集下该成员 score 值之和
	 * @param dst {const char*} 目标有序集键值
	 * @param keys 源有序集键值-权重集合；使用权重选项，可以为 每个 给定有序集 分别
	 *  指定一个乘法因子(multiplication factor)，每个给定有序集的所有成员的 score
	 *  值在传递给聚合函数(aggregation function)之前都要先乘以该有序集的因子；
	 *  如果没有指定 WEIGHTS 选项，乘法因子默认设置为 1
	 * @param aggregate {const char*} 聚合方式，默认是 SUM 聚合方式，聚合方式如下：
	 *  SUM: 将所有集合中某个成员的 score 值之 和 作为结果集中该成员的 score 值
	 *  MIN: 将所有集合中某个成员的 最小 score 值作为结果集中该成员的 score 值
	 *  MAX: 将所有集合中某个成员的 最大 score 值作为结果集中该成员的 score 值
	 * @return {int} 新保存到目标有序集的结果集中的元素(成员)数量，如果源有序集
	 *  集合中存在相同的成员，则只新增一个成员
	 */
	int zunionstore(const char* dst, const std::map<string, double>& keys,
		const char* aggregate = "SUM");

	int zunionstore(const char* dst, const std::vector<string>& keys,
		const std::vector<double>* weights = NULL,
		const char* aggregate = "SUM");

	/**
	 * 计算给定的一个或多个有序集的交集，其中给定 key 的数量必须以 numkeys 参数指定，
	 * 并将该并集(结果集)储存到目标有序集; 默认情况下，结果集中某个成员的 score
	 * 值是所有给定集下该成员 score 值之和
	 * @return {int} 新保存到目标有序集的结果集中的元素(成员)数量
	 */
	int zinterstore(const char* dst, const std::map<string, double>& keys,
		const char* aggregate = "SUM");

	int zinterstore(const char* dst, const std::vector<string>& keys,
		const std::vector<double>* weights = NULL,
		const char* aggregate = "SUM");

	/**
	 * 当有序集合的所有成员都具有相同的分值时， 有序集合的元素会根据成员的字典序
	 * （lexicographical ordering）来进行排序， 而这个命令则可以返回给定的
	 * 有序集合键 key 中， 值介于 min 和 max 之间的成员
	 * @param min {const char*} 区间最小值
	 * @param max {const char*} 区间最大值
	 * @param offset {const int*} 非空时有效，从结果集中选取的下标起始值
	 * @param count {const int*} 非空时有效，从结果集中的指定下标位置起选取的数量
	 * @return {int} 结果集中成员的数量
	 *  0: 表示结果集为空或 key 不存在
	 * -1: 表示出错或 key 对象非有序集对象
	 * >0: 结果集的数量
	 * 注：关于区间的选择规则如下：
	 * 1）合法的 min 和 max 参数必须包含 ( 或者 [ ， 其中 ( 表示开区间（指定的值
	 *   不会被包含在范围之内）， 而 [ 则表示闭区间（指定的值会被包含在范围之内）
	 * 2）特殊值 + 和 - 在 min 参数以及 max 参数中具有特殊的意义， 其中 + 表示
	 *   正无限， 而 - 表示负无限。因此，向一个所有成员的分值都相同的有序集合发送命令
	 *   ZRANGEBYLEX <zset> - + ， 命令将返回有序集合中的所有元素
	 */
	int zrangebylex(const char* key, const char* min, const char* max,
		std::vector<string>& out, const int* offset = NULL,
		const int* count = NULL);

	/**
	 * 于一个所有成员的分值都相同的有序集合键 key 来说， 这个命令会返回该集合中， 
	 * 成员介于 min 和 max 范围内的元素数量
	 * @return {int}
	 */
	int zlexcount(const char* key, const char* min, const char* max);

	/**
	 * 对于一个所有成员的分值都相同的有序集合键 key 来说， 这个命令会移除该集合中，
	 * 成员介于 min 和 max 范围内的所有元素
	 * @return {int}
	 */
	int zremrangebylex(const char* key, const char* min, const char* max);

	/**
	 * 命令用于迭代有序集合中的元素（包括元素成员和元素分值）
	 * @param cursor {int} 游标值，开始遍历时该值写 0
	 * @param out {std::map<string, double>&} 结果集
	 * @param pattern {const char*} 匹配模式，glob 风格，非空时有效
	 * @param count {const size_t*} 限定的结果集数量，非空指针时有效
	 * @return {int} 下一个游标位置，含义如下：
	 *   0：遍历结束
	 *  -1: 出错
	 *  >0: 游标的下一个位置
	 */
	int zscan(const char* key, int cursor, std::map<string, double>& out,
		const char* pattern = NULL, const size_t* count = NULL);

private:
	int zrange_get(const char* cmd, const char* key, int start,
		int stop, std::vector<string>& result);
	int zrange_get_with_scores(const char* cmd, const char* key, int start,
		int stop, std::vector<std::pair<string, double> >& out);
	int zrangebyscore_get(const char* cmd, const char* key,
		const char* min, const char* max, std::vector<string>& out,
		const int* offset = NULL, const int* count = NULL);
	int zrangebyscore_get_with_scores(const char* cmd,
		const char* key, const char* min, const char* max,
		std::vector<std::pair<string, double> >& out,
		const int* offset = NULL, const int* count = NULL);
	int zstore(const char* cmd, const char* dst,
		const std::map<string, double>& keys, const char* aggregate);
	int zstore(const char* cmd, const char* dst, const std::vector<string>& keys,
		const std::vector<double>* weights, const char* aggregate);
};

} // namespace acl
