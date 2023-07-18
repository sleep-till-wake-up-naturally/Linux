#include <iostream>
#include <cstdio>

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

void SetNonBlock(int fd)
{
    int fl = fcntl(fd,F_GETFL);//根据文件描述符获取文件属性
    if(fl < 0){
        perror("fcntl");
        return ;
    }
    
    fcntl(fd,F_SETFL, fl | O_NONBLOCK);
} 


int main()
{
    SetNonBlock(0);
    char buffer[1024];
    fd_set *readfds;
    while(1)
    {
        std::cout << "请输入>";
        fflush(stdout);
        sleep(5);//等待输入
        int size = read(0,buffer,sizeof(buffer)-1); //从标准输入中读取数据
        buffer[size-1] = 0;//将'\n'清掉
        if(size > 0){ //读取到数据
            std::cout << "echo : " << buffer << std::endl; 
        }else if(size == 0){ //读取到文件尾端
            std::cout << "read over" << std::endl;
            break;
        }else {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "等待数据准备就绪" << std::endl;
            }
            else if(errno == EINTR) continue;
            else return -1;
        }
        std::cout << std::endl;
    }

    return 0;
}
