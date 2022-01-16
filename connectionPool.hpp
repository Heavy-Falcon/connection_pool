#ifndef __CONNECTIONPOOL_HPP__
#define __CONNECTIONPOOL_HPP__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include "connection.hpp"

#endif

using namespace std;

class ConnectionPool {
public:
	shared_ptr<Connection> getConnetcion();  // 从池中取一个连接

	// 获取唯一的连接池对象
	static ConnectionPool* getInstance();

    // 删除连接池对象，解决内存泄漏问题
    static void deleteInstance();

private:
	ConnectionPool();  // 单例模式
    ~ConnectionPool();
    ConnectionPool(const ConnectionPool&);
    const ConnectionPool& operator=(const ConnectionPool&);

	bool loadConfig();
	void produceConnectionTask();
	void scannerConnectionTask();

    bool On = false;
    thread t[2];
    static atomic<ConnectionPool*> p;  // 用 原子变量 + DCL 解决线程安全的问题
    static mutex mtx;

	string _ip;
	int _port;
	string _username;
	string _password;
	string _dbname;
	int _initSize;  // 连接池连接初始量
	int _maxSize;  // 连接池最大连接量
	int _maxIdleTime;  // 连接池最大空闲时间
	int _connectionTimeOut;  // 最大超时时间
 
	queue<Connection*> _ConnectionQueue;  // 表示连接池
	mutex _queueMutex;  // 维护连接队列的线程安全互斥锁
	atomic_int _connectionCnt;  // 记录所创建的connection连接的总个数
	condition_variable empty;			
	condition_variable not_empty;
};
