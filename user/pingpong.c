#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
//你一开始竟然还怀疑是要创造一个pipe还是两个。。
//事实上上面这个问题还是值得思考一下的
//man 7 pipe之后，看到了unidirectional，懂？
//还有关于读/写条件的问题，看了手册之后也比单纯地扣书本上的定义要清晰的多m，内心也要安定的多kk
int main(void)
{
    int pipe_1[2];
    int pipe_2[2];
    pipe(pipe_1);
    pipe(pipe_2);
    char buf[2];
    int pid = fork();
    if (pid == 0) {
        
        //step 2
        read(pipe_1[0], buf, 1);//知道pipe_1的write端关闭它才会执行
        printf("child receive %c\n", *buf);

        //s 3
        printf("child receive ping\n");
        write(pipe_2[1], "b", 1);
        close(pipe_2[1]);


    }
    else {
        //step1,父进程写1byte至pipe_1
        
        write(pipe_1[1], "a", 1);
        close(pipe_1[1]);
        

        //s 4
        
        
        read(pipe_2[0], buf, 1);//直到pipe_2的write端关闭它才会执行
        printf("parent receive %c\n", *buf);
        printf("child receive pong\n");

    }


    exit(0);



}
