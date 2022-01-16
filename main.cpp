#include "connectionPool.hpp"

void test(ConnectionPool* p) {
    // 普通连接
    // Connection conn;
    // char sql[1024] = {};
    // sprintf(sql, "INSERT INTO pet VALUES ('Puffball','Diane','hamster','f','1999-03-30',NULL);");
    // conn.connect("localhost", 3306, "root", "yourpasswd", "test");
    // conn.update(sql);

    for (int i = 0; i < 125; i++) {
        shared_ptr<Connection> conn = p->getConnetcion();  // 从池中取一个连接
        char sql[1024] = {};
        sprintf(sql, "INSERT INTO pet VALUES ('Puffball','Diane','hamster','f','1999-03-30',NULL);");
        // if (conn->connect("localhost", 3306, "root", "yourpasswd", "test")) 
        conn->update(sql);
    }
}

int main() {
    clock_t begin = clock();

    // shared_ptr<Connection> conn = p.load()->getConnetcion(); // 从池中取一个连接
    // char sql[1024] = {};
    // sprintf(sql, "INSERT INTO pet VALUES ('Puffball','Diane','hamster','f','1999-03-30',NULL);");
    // // if (conn->connect("localhost", 3306, "root", "yourpasswd", "test")) 
    // //     printf("连接失败\n");
    // conn->update(sql);

    // 单线程测试
#if 0
    for (int i = 0; i < 100; i++) test();
    cout << "500次数据库插入操作用时: " << (double)(clock() - begin) / CLOCKS_PER_SEC << "s" << endl;
#endif

    // 多线程测试
    ConnectionPool* p = ConnectionPool::getInstance();

    // 创建 4 个子线程
    thread t[4];
    for (int i = 0; i < 4; i ++ )
        t[i] = thread(test, p);

    for (int i = 0; i < 4; i ++ )
        t[i].join();
    // 子线程全部结束

    cout << "500次数据库插入操作用时: " << (double)(clock() - begin) / CLOCKS_PER_SEC << "s" << endl;
    ConnectionPool::deleteInstance();  // 这个函数不能结束

    return 0;
}