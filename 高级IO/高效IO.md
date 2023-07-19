# **高级IO**

在IO过程中，存在以下两种情况：

- 当资源未准备就绪时，将会进行阻塞/非阻塞等待。
- 当资源准备就绪时，将进行数据拷贝。

也即是说，**IO的本质是进行资源的等待与数据的拷贝**。

其中，数据的拷贝完全依赖于硬件设备的性能。而如何在软件层面提高IO效率，则取决于如何减少IO在进行资源等待的时间比重。



## 五种IO模型

通过下述例子，来引出五种IO模型：

故事里有五个主人公，分别为**A、B、C、D、E**，他们相约一起去河边进行钓鱼趣味竞赛，下面是他们个人的钓鱼工具以及当天的钓鱼操作：

​	A: 准备一根鱼竿（附带鱼漂），若干鱼饵，两耳不闻窗边事的钓鱼，鱼不上钩就一直一定不动的盯着鱼漂。

​	B: 准备了一根鱼竿（附带鱼漂），若干鱼饵，一边刷着手机一边钓鱼，每过3分钟瞧一眼看看鱼漂是否有动静。

​	C: 准备一根鱼竿（附带鱼漂），若干鱼饵，一只铃铛，将铃铛挂在鱼竿头上，刷着手机头也不抬。铃铛一响立刻起鱼竿。

​	D:准备五根鱼竿（附带鱼漂），若干鱼饵，一当发现其中一个鱼漂有动静，就抬起鱼竿抓鱼。

​	E:是一个大老板，因为公司出了事情需要处理，将一根鱼竿交给秘书进行处理，秘书钓完鱼后打电话通知E，E再过来领取。

上面五个人如果要比较谁的钓鱼效率最高，那么肯定是D，原因是其所拥有的鱼竿最多，每条鱼在选择鱼饵咬钩时的概率就比其他人高。

实际上，A、B、C三人其钓鱼动作的性质是一样的，等待鱼咬钩的概率也是一样的，也就是说其效率是相等的。

如何理解E的钓鱼行为呢？E是将钓鱼任务分配给其他人，自己并不参与其中，当任务完成后，他人会告诉自己已经完成任务了。

我们可以将A、B、C、D 四人亲身参与进钓鱼任务的行为称为同步，而将E不参与进钓鱼任务的行为称为异步。

对应IO的模型来说，进程参与进等待IO资源并拷贝数据的行为叫同步IO。对不参与进IO而由操作系统代劳的行为叫做异步IO。



**可以将上面的例子映射到进程IO上：**

​	河 == 内核空间。

​	鱼 == 数据资源。

​	铃铛 == 信号。

​	鱼漂 == 鱼是否咬钩 == 资源准备就绪的事件。

​	鱼竿 == 文件描述符。

​	E的秘书 == 操作系统。



**A、B、C、D、E分别代表着的IO模型：**

​	A ：阻塞式IO。

​	B ：非阻塞式IO。

​	C ：信号驱动式IO。

​	D ：多路转接 / 多路复用。

​	E ： 异步IO。



### 阻塞式IO

概念：在内核将数据准备好之前，系统调用会一直等待。**所有的套接字默认都是阻塞方式。**阻塞IO是最常见的IO模型。



![阻塞式IO](C:\Users\15508\Desktop\github\Linux\高级IO\图片\阻塞式IO.png)

### 非阻塞式IO

概念：如果内核还未将数据准备好，系统调用仍然会直接返回，并且返回**EWOULDBLOCK**错误码。

非阻塞IO往往需要程序员以循环的方式反复尝试读写文件描述符，这个过程称之为**轮询**。这对CPU来说是很大的浪费，一般只在特定场景下使用。

![非阻塞式等待](C:\Users\15508\Desktop\github\Linux\高级IO\图片\非阻塞式等待.png)





### 信号驱动IO

概念：内核将数据准备好的时候，使用**SIGIO**信号通知应用进程进行IO。



![信号驱动IO](C:\Users\15508\Desktop\github\Linux\高级IO\图片\信号驱动IO.png)



### 多路转接

概念：虽然从流程图上看起来和阻塞IO类似。但实际上最核心在于IO多路转接能够同时等待多个文件描述符的就绪状态。



![多路转接IO](C:\Users\15508\Desktop\github\Linux\高级IO\图片\多路转接IO.png)





注意：多路转接是将IO两部分分开进行处理，**select只进行资源等待**，recvfrom只进行数据拷贝。



### 异步IO

概念：由内核在数据拷贝完成时，通知应用程序（信号驱动IO是告诉应用程序何时可以开始拷贝数据）。



![异步IO](C:\Users\15508\Desktop\github\Linux\高级IO\图片\异步IO.png)

### 小结

​	任何IO过程中，都包含两个步骤：第一是**等待**，第二是**拷贝**，而且在实际的应用场景中，等待消耗的时间往往都远高于拷贝的时间。如何让IO更高效，最核心的办法就是**让等待的时间尽可能的少**。



## 几个重要概念的区分

### 同步通信 VS 异步通信

同步与异步关注的是消息通信机制。

- 所谓同步，就是在发出一个**调用**时，在没有得到结果之前，该调用就不会**返回**。但是一旦调用返回，就得到返回值了。换句话说，就是由**调用者**主动等待这个**调用**的结果。就如钓鱼例子中的**A、B、C、D**。
- 异步则是相反，**调用**发出后，这个调用就直接返回了，所以没有返回结果。换句话说，当一个异步过程调用发出后，调用者不会立刻得到结果。而在**调用**发出后，**被调用者**通过状态、通知来通知调用者，或通过回调函数处理这个调用。就如钓鱼例子中的E，其中E是调用发起者，E的秘书就是被调用者。



### 进程/线程同步 VS 同步通信

这两者的关系完全就是老婆与老婆饼的关系，完全够不着边。

- 进程/线程同步时进程/线程之间直接的制约关系。
- 这些线程要求在访问某些临界资源时，确保这些线程按照指定顺序在某一时刻只有一个线程能够访问这个临界资源。



在未来如果看到"同步"，我们需要先搞清楚大背景是什么。这个同步，究竟是同步通信、异步通信的同步，还是同步与互斥的同步。



### 阻塞 VS 非阻塞

阻塞和非阻塞关注的是程序等待调用结果（消息，返回值）时的状态。

- 阻塞调用是指调用结果返回之前，当前线程/进程会被挂起，调用线程/进程只有在得到结果之后才会返回。
- 非阻塞调用指在不能立刻得到结果之前，该调用不会阻塞当前线程/进程。



## 非阻塞IO

一个文件描述符，默认情况下都是阻塞式的。

在以下情况中，可以手动设置IO为非阻塞状态：

- recvfrom / recv 中的flags参数设置为 **MSG_DONTWAIT**；
- sendto / send 中的flags参数设置为 **MSG_DONTWAIT**；
- 调用**fcntl**直接将文件描述符设置为非阻塞。



前两种设置只能将特定的I/O操作设置为非阻塞式，并不全面、通用。如果要将I/O两个操作都进行非阻塞设置，只需要将文件描述符设置为非阻塞即可，这需要使用到**fcntl**,该操作能一步到位。下面着重介绍 **fcntl**函数。



### fcntl

函数原型如下：

```c++
#include <unistd.h>
#include <fcntl.h>

int fcntl(int fd,int cmd,... /* arg */);
```

传入的cmd的值不同，后面追加的参数也不同。

fcntl函数有5种功能：

- 复制一个现有的描述符 （cmd=F_DUPFD）。
- 获得/设置文件描述符标记（cmd=F_GETFD 或 F_SETFD）。
- 获得/设置文件状态标记（cmd=F_GETFL 或 F_SETFL）。
- 获得/设置异步I/O所有权（cmd=F_GETOWN 或 F_SETOWN）。
- 获得/设置记录锁（cmd=F_GETLK,F_SETLK或F_SETLKW）。

这里只需要使用第三种功能，**获取/设置文件状态标记**，就可以将一个文件描述符设置为非阻塞。



### 实现函数SetNonBlock

基于**fcntl**,可以实现一个**SetNonBlock**函数，将文件描述符设置为非阻塞。

```c++
void SetNonBlock(int fd)
{
    int fl = fcntl(fd,F_GETFL);//根据文件描述符获取文件属性
    if(fl < 0){
        perror("fcntl");
        return ;
    }
    
    fcntl(fd,F_SETFL, fl | O_NONBLOCK);
} 
```

- 使用F_GETFL将当前文件描述符所对应的文件属性取出来（这是一个位图）。
- 然后再使用F_SETFL将文件描述符设置回去，设置回去的同时，将**O_NONBLOCK**设置进位图中。



### 以轮询方式读取标准输入

```c++
int main()
{
    SetNonBlock(0);
    char buffer[1024];
    while(1)
    {
        std::cout << "请输入>";
        fflush(stdout);//清空缓冲区
        sleep(5);//等待输入，防止输入和输出混杂在一起
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
```

- 先将**标准输入**设置为非阻塞式，再从标准输入中读取数据拷贝进指定空间。
- 在轮询过程中，如果资源未准备就绪，read会直接返回并**设置错误码**。
- 当**read**返回正数时，表示当前标准输入中有数据可读。
- 当**read**返回0时，表示当前已经读到文件结束标志，没有数据可以读了。
- 当**read**返回-1时，可能存在以下情况：
    1. 当前需要等待数据准备就绪，则系统会设置错误码为**EAGAIN**或者**EWOULDBLOCK**。
    2. 在阻塞读取时被信号所中断，需要重新读取，则系统会设置错误码为**EINTR**。
    3. 当前权限不够，无法打开该文件等原因，我们将这些错误视为直接错误，直接退出IO。



代码运行效果如下：

![非阻塞式IO效果图](C:\Users\15508\Desktop\github\Linux\高级IO\图片\非阻塞式IO效果图.png)



## IO多路转接之select

### 初始select

IO系统调用中的recv/recvfrom/send/sendto是一次**等待一个**文件描述符中的数据准备就绪并进行数据拷贝，其完成了IO的**等+数据拷贝**。而select则是一次**等待多个**文件描述符中的数据准备就绪，但**不进行数据拷贝**，数据拷贝操作交给read/write来完成。



### select函数原型

```C++
#include <sys/select.h>
int select(int nfds,fd_set *readfds,fd_set *writefds,fd_set *exceptfds,struct timeval *timeout);
```

**参数解释：**

- nfds : 需要监视的最大文件描述符值+1。
- readfds : 一张位图，作为输入输出型参数传入。作为输入/输出型参数时，其意义各不相同：
    - 当作为输入型参数时，其意义为：
        - 用户告诉操作系统，位图中对应的文件描述符哪些读事件需要被你关心。
        - 其位图的位置对应文件描述符的数值，位图的数值则表示是否需要操作系统关心对应文件的读事件。
        - 例如位图1001,表示对应的0号和3号文件描述符的读事件需要被操作系统关心。
    - 当作为输出型参数时，其意义为：
        - 内核告诉用户，你要关心的多个文件描述符中，有哪些的读事件已经准备就绪了。
        - 其位图的位置对应文件描述符的数值，位图的数值则表示哪些文件描述符的读事件已经就绪。
        - 例如位图0001，表示用户关心的0号文件描述符的读事件已就绪。
- writefds ：与readfds同理，但表示的是写事件。
- exceptfds ：与readfds同理，但表示的是异常事件。
- timeout ：输入输出型参数，表示阻塞等待时间。当取不同值时意义不同：
    - NULL : select进行阻塞等待。
    - {0,0} ：select进行非阻塞等待。
    - {s，m} ：s表示秒，m表示微秒。{s,m}表示先阻塞等待s秒m微秒，如果超过s秒m微秒，则非阻塞返回一次。如果在阻塞等待阶段就已经返回，则操作系统对其进行写入，修改其为剩余等待时间。



**注意：**select函数最为关键的是中间3个参数，因为select未来关心的事件也只有3种：a.读事件、b.写事件、c.异常事件。对于任意fd,都是这三种。系统通过一个**位图结构**fd_set来表示fd集合，**利用三张位图的输入和输出，用户和操作系统就能做到互相沟通，知晓对方所关心的**。



操作系统提供了一组操作**fd_set**的接口，来实现对位图的操作：

```C++
void FD_CLR(int fd, fd_set *set); //用来清除描述词组set中相关fd的位
int FD_ISSET(int fd,fd_set *set); //用来测试描述词组set中相关fd的位是否为真
void FD_SET(int fd, fd_set *set); //用来设置描述词组set中相关fd的位
void FD_ZERO(fd_set *set); //用来清除描述词组set中的全部位
```



**关于select的函数返回值**

- 返回值大于0，则表示有多少个文件描述符状态改变。
- 返回值为0，则表示在超过了timeout时间后，没有文件描述符状态改变。
- 当有错误发生时则返回-1，错误原因存于errno,此时参数readfds,writefds,exceptfds和timeout的值都是不可预测的。错误值可能为：
    - EBADF : 文件描述符为无效或该文件已经关闭。
    - EINTR : 此调用被信号所中断。
    - EINVAL : 参数nfds为负值。
    - ENOMEM : 核心内存不足。



## 实现简易select读事件服务器



如果我们要实现一个简易的select读事件服务器，该服务器要做的就是读取客户端发来的数据并进行打印，那么该服务器的大致框架如下：

- 先创建套接字，绑定套接字，并监听该套接字。
- 创建一个队列**fd_array**来管理监听套接字和与客户端连接的套接字。管理这个队列本质就是管理**select**中的读事件位图**readfds**，由于起始时只有监听套接字，所以在一开始队列中只有监听套接字。
- 在调用**select**之前，先将队列中的**fd**设置进**readfds**。然后调用**select**，等待套接字资源准备就绪，如果就绪，则进行后续处理。
- 当**select**检测到有资源准备就绪了，就会将已经就绪的文件描述符设置进**readfds**中，通过被设置进的文件描述符我们就能做不同的处理。
    - 如果读事件就绪的是监听套接字，就可以执行**accept**来获得新的连接套接字**sock**,并将**sock**添加进套接字队列**fd_array**中。
    - 如果读事件就绪的是连接套接字，就可以执行**read**操作来获得客户端数据，并进行打印。



**需要注意：**

- 监听套接字也是可以被**select**的，在监听套接字**accept**前，客户端是需要向服务器发起**三次握手**的，**三次握手可以视作客户端向服务器发送资源，而服务器也在等待这种资源并要进行读取**。所以，当三次握手后，监听套接字就处于**连接就绪状态**，这种状态被归类为**读事件就绪状态**。
- 由于fd_array是管理所有套接字的队列，并且它与**readfds**是关联起来的，那么其所要容纳的套接字数量就必须与**readfds**所表示的**最大fd值**对应。这里可以通过**sizeof**求得**readfds**的大小，但由于**readfds**是一张位图，**每个比特位都对应一个fd值**，所以**sizeof求的大小要乘8**才算是fd_array所要容纳的套接字数量。



### 封装socket类

```C++
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
```

**注意：**

​	在某些情况下，服务器会关闭与客户端的连接。而由于在四次挥手中，主动关闭连接的一方，最后会处于**TIME_WAIT**状态，需要等待2MSL（一般为120s）才能重新**bind**端口号。如果不想要等待这个时间，就需要**setsockopt**来清除掉关闭连接后的**TIME_WAIT**状态。



### SelectServer服务器

SelectServer的成员：

- 创建一个fd_array队列，大小由readfds求得。每个成员对应新的fd，初始时全为非法fd。
- 服务器运行起来时的端口号，后续进行bind时使用。
- 管理监听套接字，后续服务器启动时，用于获得新连接。
- 一个回调函数对象，用于在获取数据后对数据进行处理。

```C++
class SelectServer
{
public:
	static const int fdnum = sizeof(fd_set) * 8;//fd_array所要容纳的合法fd
	static const int defaultfd = -1; //非法fd
    using func_t = function<std::string(std::const string)>;

	SelectServer(func_t f, uint16_t port = 8080)
		:_port(port),fd_array(nullptr),func(f)
		{}
		
		//TODO
	
private:
	int *fd_array;
	uint16_t _port;
	int _listensock;
    func_t func;
}
```



在服务器启动之前，需要先将服务器进行初始化，这个初始化包括了创建监听套接字，并绑定该套接字，监听该套接字。

此时只有**listensock**这一个套接字，我们需要将它放进**fd_array**中进行管理。

```C++
void init()
{
    fd_array = new int[fdnum];
    _listensock = Sock::Socket();
    Sock::Bind(_listensock,_port);
    Sock::Listen(_listensock);
    //初始化套接字集合
    for(int i = 0; i < fdnum; i++)
    {
    	fd_array[i] = defaultfd;
    }
    //此时只有listensock套接字
    fd_array[0] = _listensock;
}
```



在服务器启动之后，希望能够通过**select**来等待**现有套接字**中的资源就绪，一旦资源就绪，就调用特定处理函数，对套接字资源进行处理。

```C++
void start()
{
	while(1)
	{
		fd_set *readfds;//读事件
		int maxfd = fd_array[0];//select第一个参数
        //将现有的所有合法fd都设置进readfds
		for(int i = 0; i < fdnum; i++)
		{
			if(fd_array[i] != defaultfd)
			{
				//如果此时的fd为合法，就设置进readfds
				FD_SET(fd_array[i],readfds);
                //更新最大的fd数值
                if(maxfd < fd_array[i])		maxfd = fd_array[i];
			}
			else continue; //非法fd就跳过
		}
     	
        timeval timeout = {1,0};//每次select阻塞等待1s
        //select进行等待套接字资源就绪
        int n = select(maxfd,readfds,nullptr,nullptr,&timeout);
        
        //TODO
	}
}
```

- 因为服务器基本上是不会退出的，所有要不断循环的使用select来等待套接字资源并进行处理。
- 在第一次循环时，**fd_array**中只有**listensock**这一个套接字。而在后面的循环中，**listensock**可以**"衍生"**出其他**连接套接字**，这些套接字我们都需要关心它们的读事件。所以在每次select之前，我们都需要检查是否有新的连接套接字产生。如果有，需要将它们设置进**readfds**来告诉操作系统我们所要关心的读事件。
- **select**的第一个参数要求传入目前最大的fd数值，所以在检查是否有套接字生成的时候，也要判断**maxfd**是否需要修改。
- 如果设置了**timeout**参数，那么我们需要将它放在循环内部。因为在每次**select**时，**timeout**中的数值都会被修改。如果放在循环外部，当**timeout**里面的时间都被消耗完，select就会变为非阻塞等待（timeout = {0,0} ）。



由于设置的是非阻塞等待，所以每次**select**调用完后会立即返回，我们可以根据这个返回值做出不同的处理动作。代码接上文：

```C++
switch(n)
{
case 0: //超出阻塞时间，直接返回
    logMessage(NORMAL,"timeout...");
    break;
case -1: //select错误
    logMessage(FATAL,"select error,code : %d",errno);
     break;
default: //有资源准备就绪
     logMessage(NORMAL,"have event ready");
     //HandlerReadEvent(readfds);
     break;
}
```

- 返回值为0时，说明超出阻塞等待时间后，仍然没有套接字资源准备就绪。
- 返回值为-1时，说明**select**调用出错。
- 返回值大于0时，说明已经有套接字资源准备就绪，此时需要根据套接字是监听套接字还是连接套接字分开进行处理。



select调用结束后，如果返回值大于0，则操作系统会修改**readfds**来告诉我们哪些**fd**的读事件已经就绪，我们对这些已就绪的读事件进行处理。

```C++
void HandlerReadEvent(fd_set *readfds)
{
    for (int i = 0; i < fdnum; i++)
    {
        // 检测哪些读事件就绪
        if (fd_array[i] == defaultfd)
            continue;

        if (FD_ISSET(fd_array[i], readfds) && fd_array[i] == _listensock)
        {
            // 监听套接字处理方式
            Accepter(_listensock);
        }
        else
        {
            //连接套接字处理方式
            Recver(fd_array[i],i);
        }
    }
}
```



对于监听套接字，便调用**accept**来获取连接套接字，并将这些连接套接字管理起来。

```C++
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
```

- 由于**select**等资源准备就绪后才告知的我们，所以当我们调用**accept**的时候，是不需要进行阻塞等待的。
- 当获取到套接字后，不要直接调用**read/recv**函数，因为**read/recv**同样需要进行**等+数据拷贝**的过程。我们将**read/recv等**的这个过程同样交给**select**来处理。所以在获取连接之后，只需要将连接套接字管理进fd_array中即可。
- 由于一个**readfds**最多承载**sizeof(readfds) * 8**个fd，所以当连接**fd**大于这个数时，只能暂时关闭掉那些**fd**大于这个数的连接。



对于连接套接字，便调用write函数来读取从**client**端获取的数据，并通过自定义回调函数处理这些数据并响应给**client**端。

```
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
```

- 如果采用的是**tcp**协议，那么我们**读取到的报文可能并不是一个完整的报文**。单次读取可能会造成读取不完整的情况，但是如果用循环，**在第二次及以上的次数中，recv便需要再进行阻塞等待**，这不是我们想要看到的。所以上述代码是有bug的，我们这点在epoll上再进行修改。
- 当**client**端突然断开连接，或者要读取的**client**端已经关闭，我们都需要手动关闭连接，并将对应**fd**移出**fd_array**队列。



### select的缺点

- 每次调用**select**,都需要手动设置**fd**集合，从接口使用角度来说这非常不便。
- 每次调用**select**，都需要把**fd**集合从用户态拷贝到内核态，这个开销在**fd**很多时会非常大。
- 同时每次调用**select**都需要在内核从0开始遍历传递进来的所有**fd**，直至最大的**maxfd**，这个开销在**fd**很多时也很大。
- **select**支持的文件描述符数量太小。







