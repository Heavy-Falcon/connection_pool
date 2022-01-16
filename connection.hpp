#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__

#include <cstdlib>
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <time.h>

#endif

#define LOG(str) cout << __FILE__ <<  ":" << __LINE__ << ": " << str << endl;

using namespace std;
// 数据库操作类 
class Connection {
public:
	// 初始化数据库连接 
	Connection();
 
	// 释放数据库连接资源 
	~Connection();
	
	// 连接数据库
	bool connect(string ip, unsigned int port, string user, string password, string dbname);
	
	// 更新操作 insert、delete、update 
	bool update(string sql);
 
	// 查询操作 select 
	MYSQL_RES* query(string sql);
 
	//刷新连接的起始空闲时间
	void refreshAlivetime();
 
	//返回存活时间
	clock_t getAliveTime();
 
private:
	MYSQL *_conn;  // 表示和MySQL Server的一个连接 
	clock_t _alivetime;
};
