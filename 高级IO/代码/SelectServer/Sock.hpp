#pragma once
#include "log.hpp"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BACKLOG 32

class Sock
{
public:
    static int Socket()
    {
        //创建socket套接字对象
        int sock = socket(AF_INET,SOCK_STREAM,0);
        if(sock < 0){
            logMessage(FATAL,"create socket fail");
            exit(SOCKET_ERR);
        }
        logMessage(NORMAL,"create socket success : %d",sock);

        int opt = 1;
        setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

        return sock;
    }

    static void Bind(int sock,int port)
    {
        //绑定自己的网络信息
        struct sockaddr_in local;
        memset(&local,0,sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = INADDR_ANY;
        if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
        {
            logMessage(FATAL,"server bind fail");
            exit(BIND_ERR);
        } 
        logMessage(NORMAL,"server bind success");
    }

    static void Listen(int sock)
    {
        //监听套接字
        int n = listen(sock,BACKLOG);
        if(n < 0){
            logMessage(FATAL,"server listen fail");
            exit(LISTEN_ERR);
            
        }
        logMessage(NORMAL,"server listen success");
    }

    static int Accept(int listensock,uint16_t *clientport,std::string* clientip)
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        //获取连接
        int sock = accept(listensock,(struct sockaddr*)&peer,&len);
        if(sock < 0){
            logMessage(WARNING,"server accept fail,next");
        }else {
            logMessage(NORMAL,"server accept success");
            *clientport = ntohs(peer.sin_port);
            *clientip = inet_ntoa(peer.sin_addr);
        }

        return sock;
    }
};