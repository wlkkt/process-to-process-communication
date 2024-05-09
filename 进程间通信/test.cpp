#include <iostream>
#include <unistd.h>
#include <cerrno>  //c++版本的errno.h
#include <cstring> //c++版本的string.h
#include <sys/wait.h>
#include <sys/types.h>
#include <string>

// 携带发送的信息
std::string getOtherMessage()
{
    // 获取要返回的信息
    static int cnt = 0; // 计数器
    std::string messageid = std::to_string(cnt);
    cnt++;                    // 每使用一次计数器就++
    pid_t self_id = getpid(); // 获取当前进程的pid
    std::string stringpid = std::to_string(self_id);

    std::string message = " my messageid is : ";
    message += messageid;
    message += " my pid is : ";
    message += stringpid; // 逐渐向要传回的string字符串中追加要返回的信息

    return message;
}

// 子进程进行写入
void ChildProcessWrite(int wfd)
{
    std::string message = "father, I am your child process!";
    while (true)
    {
        std::string info = message + getOtherMessage(); // 子进程尝试向父进程传递的所有信息
        write(wfd, info.c_str(), info.size());          // write函数传入的字符串需要是c语言格式的，c_str将string字符串变为c语言格式的字符串
        sleep(1);                                       // 让子进程写慢一点，这样父进程就不会一直读并打印在显示器上
    } // write是由操作系统提供的接口，而操作系统又是C语言编写的，所以后续学习中可能会碰到c语言的接口和c++的接口混合使用的情况
} // info最后有/0但是文件不需要

const int size = 1024; // 定义父进程可以读取的数组大小

// 父进程进行读取
void FatherProcessRead(int rfd)
{
    char inbuffer[size]; // 普通的c99标准不支持变长数组，但是这里使用的是gnb的c99标准，gun的c99标准支持变长数组
    while (true)
    {
        ssize_t n = read(rfd, inbuffer, sizeof(inbuffer)); // 因为文件不需要\0,所以读取管道中内容到缓冲区时可以少读取一个并将/0变为0
        if (n > 0)
        {
            inbuffer[n] = 0;
            std::cout << "父进程获取的消息: " << inbuffer << std::endl;
        }
    }
}

int main()
{
    // 1、创建管道
    int pipefd[2];
    int n = pipe(pipefd); // 输出型参数，rfd,wfd
    if (n != 0)
    {
        std::cerr << "errno" << errno << ":" << "errstring" << strerror(errno) << std::endl;
        return 1;
    }

    // pipefd[0]即读端fd，pipefd[1]即写端fd
    std::cout << "pipefd[0] = " << pipefd[0] << ", pipefd[1] = " << pipefd[1] << std::endl;

    sleep(1); // 便于看到管道创建成功

    // 2、创建子进程
    pid_t id = fork();
    if (id == 0)
    {
        std::cout << "子进程关闭不需要的fd,准备发消息了" << std::endl;
        sleep(1); // 便于感受到发消息的过程

        // 子进程---写端
        // 3、关闭不需要的fd
        close(pipefd[0]);

        // //将下面的代码解除注释可以验证“血缘关系”的说法，孙子进程仍然可以和爷爷进程进行通信
        // // 子进程再创建一个子进程
        // if (fork() > 0)
        // {
        //     exit(0); // 退出作为父亲的子进程
        // }
        // // 孙子进程进行的操作


        ChildProcessWrite(pipefd[1]); // 子进程的写函数

        close(pipefd[1]); // 完成通信后也将子进程的写端关闭
        exit(0);
    }
    std::cout << "发进程关闭不需要的fd,准备收消息了" << std::endl;
    sleep(1); // 便于感受到收消息的过程

    // 父进程---读端
    close(pipefd[1]);
    FatherProcessRead(pipefd[0]); // 父进程的读函数
    close(pipefd[0]);             // 完成通信后也将子进程的读端关闭

    pid_t rid = waitpid(id, nullptr, 0);
    if (rid > 0)
    {
        std::cout << "wait child process done" << std::endl;
    }

    return 0;
}