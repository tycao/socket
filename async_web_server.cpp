#include <iostream>
#include <vector>
#include <memory>
using namespace std;

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <thread>
#include <boost/bind.hpp>
using namespace boost;
using namespace boost::asio;

constexpr size_t c_bufSize = 1024 * 8;

bool g_flag = true;

//using TSockPtr = std::shared_ptr<ip::tcp::socket>;
typedef std::shared_ptr<ip::tcp::socket> TSockPtr;

class TMySock
{
public:
    TMySock(io_service &io, ip::tcp::acceptor &acc, TSockPtr sockPtr) : m_iosrv(io), m_acc(acc), m_sock(sockPtr){}
    io_service &m_iosrv;    //成员引用
    ip::tcp::acceptor &m_acc; //成员引用
    std::shared_ptr<ip::tcp::socket> m_sock;
    char m_buf[c_bufSize + 1] = {0};
};
typedef std::shared_ptr<TMySock> TMySockPtr;

//声明MyRead函数
extern void MyRead(TMySockPtr sock, const system::error_code &ec, unsigned sz);

void MyWrite(TMySockPtr sock, const system::error_code &ec, unsigned sz)
{
    cout << "output to client......Success!" << endl;
    sock->m_sock->async_read_some(buffer(sock->m_buf, c_bufSize), bind(MyRead, sock, _1, _2));
}
void MyRead(TMySockPtr sock, const system::error_code &ec, unsigned sz)
{
    if (ec)
    {
        cout << ec.message() << endl;
        return;
    }
    cout << sock->m_buf << endl;
    sock->m_sock->async_write_some(buffer("ok~~~\r\n"), bind(MyWrite, sock, _1, _2));
}

void MyWork(TMySockPtr sock)
{
    sock->m_sock->async_read_some(buffer(sock->m_buf, c_bufSize), bind(MyRead, sock, _1, _2));
}

void MyAccept(TMySockPtr sock, const system::error_code &ec)
{
    if (ec)
    {
        cout << ec.message() << endl;
        return;
    }
    //启动下一次的监听
    if (g_flag)
    {
        ip::tcp::socket *pSock = new ip::tcp::socket(sock->m_iosrv);
        TMySock *mySock = new TMySock(sock->m_iosrv, sock->m_acc, TSockPtr(pSock));
        sock->m_acc.async_accept(*pSock, bind(&MyAccept, TMySockPtr(mySock), _1));
        cout << "next" << endl;
    }

    //处理本次监听获取的客户端链接
    cout << "accept" << boost::this_thread::get_id() << ec.message() << endl
         << sock->m_sock->remote_endpoint().address() << ":"
         << sock->m_sock->remote_endpoint().port() << endl;

    sock->m_sock->write_some(buffer("welcome..."));

    //异步处理
    MyWork(sock);
    boost::this_thread::sleep(posix_time::seconds(5));
}

int main(int argc, char *argv[])
{
    asio::io_service iosrv;
    ip::tcp::endpoint ep(ip::tcp::v4(), 20174); //check the incoming ip address 检查进来的ip地址
    ip::tcp::acceptor acc(iosrv, ep);

    ip::tcp::socket *pSock = new ip::tcp::socket(iosrv);
    TMySock *mysock = new TMySock(iosrv, acc, TSockPtr(pSock));
    acc.async_accept(*pSock, bind(MyAccept, TMySockPtr(mysock), _1));

    cout << "begin" << boost::this_thread::get_id() << endl;

    std::thread thrd1([&](){ iosrv.run(); });
    std::thread thrd2([&](){ iosrv.run(); });

    char c = 0;
    do
    {
        cin >> c;
    } while( 'q' != c );
    g_flag = false;

    acc.close();


    thrd1.join();
    thrd2.join();

cout << "end" << endl;
    return 0;
}
