实现一个管道通信程序：由父进程创建一个管道，然后再创建3个子进程，并由这三个子进程用管道与父进程之间进行通信：子进程发送信息，父进程等三个子进程全部发完消息后再接收信息。通信的具体内容可根据自己的需要随意设计，要求能够实验阻塞型读写过程的各种情况，并要求实现进程间对管道的互斥访问。运行程序，观察各种情况下，进程实际读写的字节数以及进程阻塞唤醒情况。
