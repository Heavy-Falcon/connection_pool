#include "connectionPool.hpp"
#include <thread>

// 静态变量在类外的声明
atomic<ConnectionPool*> ConnectionPool::p{nullptr};
mutex ConnectionPool::mtx;

// 帮用户获取从池中取线程的函数
shared_ptr<Connection> ConnectionPool::getConnetcion() {
    while (_ConnectionQueue.empty()) {
        unique_lock<mutex> lock(_queueMutex);
        // 如果队列为空，就等待，等 队列变为不空，wait for not empty
        if (cv_status::timeout == not_empty.wait_for(lock, chrono::milliseconds(_connectionTimeOut))) {  // 表示因为超时返回
            LOG("获取空闲连接超时了...获取连接失败!");
            return nullptr;
        }
    }

    // 对于shared_ptr而言，出作用域后是自动析构，而我们要的是
    // 将连接归还，所以自定义删除器，使用lamboda表达式来定义删除器
    shared_ptr<Connection> sp(_ConnectionQueue.front(), [&](Connection* conn) {
        lock_guard<mutex> lock(_queueMutex);
        conn->refreshAlivetime();
        _ConnectionQueue.push(conn);  // 归还给连接池
        _connectionCnt ++ ;
    });

    _ConnectionQueue.pop();
    _connectionCnt -- ;
    if (_ConnectionQueue.empty()) empty.notify_all(); // 通知 生产者线程 队列空了
    return sp;
}

ConnectionPool* ConnectionPool::getInstance() {
    if (p == nullptr) {
        lock_guard<mutex> lock(mtx);  // 双检测锁模式DCL (Double-Checked Locking Pattern)
        if (p == nullptr) p = new ConnectionPool;
    }
    return p;
}

void ConnectionPool::deleteInstance() {
    if (p != nullptr) {
        cout << "释放唯一的连接池ing..." << endl;
        delete p;
        p = nullptr;
    }
}

ConnectionPool::~ConnectionPool() {
    On = false;
    cout << "析构数据库连接池" << endl;
    while (_ConnectionQueue.size()) {
        Connection* conn = _ConnectionQueue.front();
        _ConnectionQueue.pop();
        _connectionCnt -- ;
        delete conn;
    }

    empty.notify_all();

    for (int i = 0; i < 2; i ++ ) t[i].join();
    cout << "所有连接释放完毕，子线程已回收" << endl;
}

// 连接池的构造函数
ConnectionPool::ConnectionPool() {
    if (!loadConfig())
        return;

    // 初始的连接
    for (int i = 0; i < _initSize; i ++ ) {
        Connection* p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        _ConnectionQueue.push(p);
        p->refreshAlivetime();
        _connectionCnt++;
    }
    On = true;

    // 生产者线程
    t[0] = thread(bind(&ConnectionPool::produceConnectionTask, this));

    // 启动一个扫描者线程，扫描超过maxIdletime时间的空闲连接，进行队列的回收
    t[1] = thread(bind(&ConnectionPool::scannerConnectionTask, this));
}

bool ConnectionPool::loadConfig() {
    FILE *pf = fopen("../mysql.ini", "r");
    if (pf == nullptr) {
        LOG("mysql.ini does not exist!");
        return false;
    }

    while (!feof(pf)) {
        char line[1024] = {};
        fgets(line, 1024, pf); // 从配置文件里读取一行到line里，再存到str里
        string str(line);
        int idx = str.find('=', 0);

        // 无效的配置项
        if (idx == str.npos)
            continue;

        int len = str.size();

        string key = str.substr(0, idx);
        string value = str.substr(idx + 1, len - idx - 2);

        if (key == "ip")
            _ip = value;
        else if (key == "port")
            _port = atoi(value.c_str());
        else if (key == "username")
            _username = value;
        else if (key == "password")
            _password = value;
        else if (key == "dbname")
            _dbname = value;
        else if (key == "initSize")
            _initSize = atoi(value.c_str());
        else if (key == "maxSize")
            _maxSize = atoi(value.c_str());
        else if (key == "maxIdexTime")
            _maxIdleTime = atoi(value.c_str());
        else if (key == "maxconnectionTimeout")
            _connectionTimeOut = atoi(value.c_str());
    }
    return true;
}

void ConnectionPool::produceConnectionTask() {
    while (On) {
        unique_lock<mutex> lock(_queueMutex);
        while (_ConnectionQueue.size()) {  // 如果队列不空，就等待，一直等队列变空，wait for empty
            empty.wait(lock);
            // 如果队列为空，就得创建新的连接
            if (On && _connectionCnt < _maxSize) {
                Connection *conn = new Connection();
                conn->connect(_ip, _port, _username, _password, _dbname);
                conn->refreshAlivetime();
                _ConnectionQueue.push(conn);  // 将新连接入队
                _connectionCnt++;
                not_empty.notify_all(); // 通知 消费者线程 队列不空了
            }
        }
    }
}


// 用来对队列进行扫描的线程函数
void ConnectionPool::scannerConnectionTask() {
    while (On) {
        // 通过sleep来模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));

        // 扫描整个队列，释放空闲连接
        unique_lock<mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize) {
            Connection *p = _ConnectionQueue.front();

            // 如果说连接的空闲时间超过了最大空闲时间，就将连接出队，然后delete
            if (p->getAliveTime() > _maxIdleTime) {
                _ConnectionQueue.pop();
                _connectionCnt--;
                delete p;  // 调用~Connection()将资源释放即可
            } else
                break;  // 如果当前元素未超时，那么该元素后面的元素也未超时
        }
    }
}