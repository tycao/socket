#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <boost/asio.hpp>
using namespace boost;
using namespace boost::asio;

int main(int argc, char *argv[])
{
    io_service iosrv;
    ip::tcp::endpoint ep(ip::tcp::v4(), 8080);
    ip::tcp::acceptor acc(iosrv, ep);

    while (true)
    {
        ip::tcp::socket sock(iosrv);
        acc.accept(sock);

        string sRequest;
        char buf[1024 * 8 + 1] = {0};
        do
        {
            memset(buf, 0, 1024 * 8);
            int len = sock.read_some(buffer(buf, 1024 * 8));
            sRequest.append(buf, buf + len);
        //} while (sRequest.substr(sRequest.size() - 4, 4) != "\r\n\r\n");
        } while (sRequest.substr(sRequest.size() - 1, sRequest.size()) != "q");
        cout << sRequest << endl;
        ifstream f("d:\\web_sohu.html", ios_base::in|ios_base::binary);
        char bufRes[1024 * 1024 + 1] = {0};
        int len = f.readsome(bufRes, 1024 * 1024);
        cout << "len : " << len << endl;
        cout << "bufRes : " << bufRes << endl;
        istringstream ss("OK");
        sock.write_some(buffer(bufRes, len));
        f.close();
    }
}
