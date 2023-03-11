#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
//你一开始竟然还怀疑是要创造一个pipe还是两个。。
//事实上上面这个问题还是值得思考一下的
//man 7 pipe之后，看到了unidirectional，懂？
//还有关于读/写条件的问题，看了手册之后也比单纯地扣书本上的定义要清晰的多m，内心也要安定的多kk
//
//ok ,事实证明一个pipe完全够用了，read所做的工作不仅仅是read，它是“读取”，会让pipe的缓冲区的n内容-1
//
int main(void)
{
    int Pipe[2];
    pipe(Pipe);
    char buf[2];
    int pid = fork();
    if (pid == 0) {
        
        //step 2
        read(Pipe[0], buf, 1);//知道pipe_1的write端关闭它才会执行
        printf("child receive %c\n", *buf);

        //s 3
        printf("child receive ping\n");
        write(Pipe[1], "b", 1);
        close(Pipe[1]);


    }
    else {
        

       //由于子进程read在前面，所以b父进程的write先执行
       write(Pipe[1],"a",1);
      close(Pipe[1]);
      wait(0);

      read(Pipe[0],buf,1);
  printf("parent receive %c\n", *buf);


      printf("parent receive pong\n");


        

    }


    exit(0);



}
