#include <iostream>
#include <unistd.h>
#include <cerrno>  //c++�汾��errno.h
#include <cstring> //c++�汾��string.h
#include <sys/wait.h>
#include <sys/types.h>
#include <string>

// Я�����͵���Ϣ
std::string getOtherMessage()
{
    // ��ȡҪ���ص���Ϣ
    static int cnt = 0; // ������
    std::string messageid = std::to_string(cnt);
    cnt++;                    // ÿʹ��һ�μ�������++
    pid_t self_id = getpid(); // ��ȡ��ǰ���̵�pid
    std::string stringpid = std::to_string(self_id);

    std::string message = " my messageid is : ";
    message += messageid;
    message += " my pid is : ";
    message += stringpid; // ����Ҫ���ص�string�ַ�����׷��Ҫ���ص���Ϣ

    return message;
}

// �ӽ��̽���д��
void ChildProcessWrite(int wfd)
{
    std::string message = "father, I am your child process!";
    while (true)
    {
        std::string info = message + getOtherMessage(); // �ӽ��̳����򸸽��̴��ݵ�������Ϣ
        write(wfd, info.c_str(), info.size());          // write����������ַ�����Ҫ��c���Ը�ʽ�ģ�c_str��string�ַ�����Ϊc���Ը�ʽ���ַ���
        sleep(1);                                       // ���ӽ���д��һ�㣬���������̾Ͳ���һֱ������ӡ����ʾ����
    } // write���ɲ���ϵͳ�ṩ�Ľӿڣ�������ϵͳ����C���Ա�д�ģ����Ժ���ѧϰ�п��ܻ�����c���ԵĽӿں�c++�Ľӿڻ��ʹ�õ����
} // info�����/0�����ļ�����Ҫ

const int size = 1024; // ���常���̿��Զ�ȡ�������С

// �����̽��ж�ȡ
void FatherProcessRead(int rfd)
{
    char inbuffer[size]; // ��ͨ��c99��׼��֧�ֱ䳤���飬��������ʹ�õ���gnb��c99��׼��gun��c99��׼֧�ֱ䳤����
    while (true)
    {
        ssize_t n = read(rfd, inbuffer, sizeof(inbuffer)); // ��Ϊ�ļ�����Ҫ\0,���Զ�ȡ�ܵ������ݵ�������ʱ�����ٶ�ȡһ������/0��Ϊ0
        if (n > 0)
        {
            inbuffer[n] = 0;
            std::cout << "�����̻�ȡ����Ϣ: " << inbuffer << std::endl;
        }
    }
}

int main()
{
    // 1�������ܵ�
    int pipefd[2];
    int n = pipe(pipefd); // ����Ͳ�����rfd,wfd
    if (n != 0)
    {
        std::cerr << "errno" << errno << ":" << "errstring" << strerror(errno) << std::endl;
        return 1;
    }

    // pipefd[0]������fd��pipefd[1]��д��fd
    std::cout << "pipefd[0] = " << pipefd[0] << ", pipefd[1] = " << pipefd[1] << std::endl;

    sleep(1); // ���ڿ����ܵ������ɹ�

    // 2�������ӽ���
    pid_t id = fork();
    if (id == 0)
    {
        std::cout << "�ӽ��̹رղ���Ҫ��fd,׼������Ϣ��" << std::endl;
        sleep(1); // ���ڸ��ܵ�����Ϣ�Ĺ���

        // �ӽ���---д��
        // 3���رղ���Ҫ��fd
        close(pipefd[0]);

        // //������Ĵ�����ע�Ϳ�����֤��ѪԵ��ϵ����˵�������ӽ�����Ȼ���Ժ�үү���̽���ͨ��
        // // �ӽ����ٴ���һ���ӽ���
        // if (fork() > 0)
        // {
        //     exit(0); // �˳���Ϊ���׵��ӽ���
        // }
        // // ���ӽ��̽��еĲ���


        ChildProcessWrite(pipefd[1]); // �ӽ��̵�д����

        close(pipefd[1]); // ���ͨ�ź�Ҳ���ӽ��̵�д�˹ر�
        exit(0);
    }
    std::cout << "�����̹رղ���Ҫ��fd,׼������Ϣ��" << std::endl;
    sleep(1); // ���ڸ��ܵ�����Ϣ�Ĺ���

    // ������---����
    close(pipefd[1]);
    FatherProcessRead(pipefd[0]); // �����̵Ķ�����
    close(pipefd[0]);             // ���ͨ�ź�Ҳ���ӽ��̵Ķ��˹ر�

    pid_t rid = waitpid(id, nullptr, 0);
    if (rid > 0)
    {
        std::cout << "wait child process done" << std::endl;
    }

    return 0;
}