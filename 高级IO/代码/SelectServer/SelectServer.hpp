#pragma once
#include "log.hpp"
#include "Sock.hpp"
#include <sys/select.h>
#include <functional>

namespace SelectServer_ns
{
    static const int fdnum = sizeof(fd_set) * 8;
    static const int defaultfd = -1;
    using func_t = std::function<std::string(const std::string)>;

    class SelectServer
    {
    public:
        SelectServer(func_t f,uint16_t port = 8080)
            : _port(port), fd_array(nullptr),func(f)
        {
        }

        void init()
        {
            fd_array = new int[fdnum];
            _listensock = Sock::Socket();
            Sock::Bind(_listensock, _port);
            Sock::Listen(_listensock);
            // 初始化套接字集合
            for (int i = 0; i < fdnum; i++)
            {
                fd_array[i] = defaultfd;
            }
            // 此时只有listensock套接字
            fd_array[0] = _listensock;
        }

        void Print()
        {
            std::cout << "fd list : ";
            for (int i = 0; i < fdnum; i++)
            {
                if (fd_array[i] != defaultfd)
                    std::cout << fd_array[i] << " ";
                else
                    continue;
            }
            std::cout << std::endl;
        }

        void Accepter(int listensock)
        {
            // listen套接字有连接资源准备就绪
            uint16_t clientport;
            std::string clientip;
            // accept == 等(三次握手) + 连接
            // 等这个过程由select来完成
            int sock = Sock::Accept(_listensock, &clientport, &clientip); // 仅进行连接
            // 将sock托管给select进行处理,也就是将sock放在fd_array上进行管理即可
            int i = 0;
            for (i = 0; i < fdnum; i++)
            {
                if (fd_array[i] == defaultfd)
                    break; // 该位置可以被占用
                else
                    continue; // 该位置不能被占用
            }

            if (i == fdnum)
            {
                // 套接字已经被设置满了
                logMessage(WARNING, "server is full,please wait");
                close(sock); // 关闭连接
            }
            else
            {
                fd_array[i] = sock; // 将sock放进fd_array上进行管理
            }
            Print(); // 打印此时的套接字队列
            logMessage(NORMAL, "accept out");
        }

        void Recver(int sock, int pos)
        {
            char buffer[1024];
            //有bug
            int n = recv(sock, &buffer, sizeof(buffer), 0);
            if (n > 0)
            { // 读取到数据
                buffer[n] = 0;
                logMessage(NORMAL, "client# %s", buffer);
            }
            else if (n == 0)
            {
                // client连接断开
                logMessage(WARNING, "client quit");
                close(sock);
                fd_array[pos] = defaultfd;
                return ;
            }
            else
            {
                // client error
                logMessage(ERROR, "client error, code : %d, string error : %s", errno,strerror(errno));
                close(sock);
                fd_array[pos] = defaultfd;
                return ;
            }

            //处理request
            std::string request = func(buffer);
            
            //响应给client
            write(sock,request.c_str(),request.size());
            
            logMessage(NORMAL,"recv out");
        }

        void HandlerReadEvent(fd_set *readfds)
        {
            for (int i = 0; i < fdnum; i++)
            {
                // 检测哪些读事件就绪
                if (fd_array[i] == defaultfd)
                    continue;

                //logMessage(DEBUG,"%d",i);
                if (FD_ISSET(fd_array[i], readfds) && fd_array[i] == _listensock)
                {
                    // 监听套接字处理方式
                    Accepter(_listensock);
                }
                else if(FD_ISSET(fd_array[i], readfds))
                {
                    Recver(fd_array[i],i);
                }
            }
        }

        void start()
        {
            while (1)
            {
                int maxfd = fd_array[0];
                fd_set *readfds;
                FD_ZERO(readfds);
                for (int i = 0; i < fdnum; i++)
                {
                    if (fd_array[i] == defaultfd)
                        continue;
                    else
                    {
                        // 将所有合法的fd设置进readfds
                        FD_SET(fd_array[i], readfds);
                        // 修改最大文件描述符数值
                        if (maxfd < fd_array[i])
                            maxfd = fd_array[i];
                    }
                }
                timeval timeout = {1, 0};
                int n = select(maxfd + 1, readfds, nullptr, nullptr, nullptr); // 非阻塞式等待
                switch (n)
                {
                case 0:
                    // 非阻塞等待下，没有资源准备就绪
                    logMessage(NORMAL, "timeout...");
                    break;
                case -1:
                    logMessage(FATAL, "select error,code : %d,string error : %s", errno, strerror(errno));
                    break;
                default:
                    // 资源准备就绪,此时只有监听套接字
                    logMessage(NORMAL, "have event ready");
                    HandlerReadEvent(readfds);
                    break;
                }
            }
        }

        ~SelectServer() {}

    private:
        uint16_t _port;
        int _listensock; // 监听套接字
        int *fd_array;   // 套接字集合
        func_t func;
    };
}