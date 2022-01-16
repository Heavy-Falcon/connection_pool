#include "connection.hpp"

using namespace std;
 
Connection::Connection() {  // 构造函数的定义
	// 初始化数据库连接
    cout << "初始化一条连接" << endl;
	_conn = mysql_init(nullptr);
}
 
Connection::~Connection() {  // 析构函数的定义
	if (_conn) {
        cout << "关闭一条数据库连接" << endl;
		mysql_close(_conn);  // 释放数据库连接资源
    }
}
 
bool Connection::connect(string ip, unsigned int port,
	string username, string password, string dbname) {
	// 连接数据库
	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p == nullptr) fprintf(stderr, "Failed to connect: Error: %s\n", mysql_error(_conn));
    cout << "建立一条到数据库的连接" << endl;
	return p != nullptr;
}
 
bool Connection::update(string sql) {
	// 更新操作 insert、delete、update
	if (mysql_query(_conn, sql.c_str())) {
		fprintf(stderr, "Failed to connect: Error: %s\n", mysql_error(_conn));
        LOG("更新失败: " + sql);
		return false;
	}
	return true;
}
 
// 刷新连接的起始空闲时间
void Connection::refreshAlivetime() {
	_alivetime = clock();
}
 
// 返回存活时间
clock_t Connection:: getAliveTime() {
	return (double)(clock() - _alivetime) / CLOCKS_PER_SEC;
}
 
MYSQL_RES* Connection::query(string sql)
{
	// 查询操作 select
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG("查询失败: " + sql);
		return nullptr;
	}
	return mysql_store_result(_conn);
}
