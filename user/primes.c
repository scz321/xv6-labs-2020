#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//只用一个pipe来让所有进程公用在xv6中暂时无法实现
//xv系统的资源有限，支持不了太多的文件描述符
//管道是比较占用资源的，如果管道的read端口和write端口都被关闭了pipe就会被释放
//fork 会将父进程的所有文件描述符都复制到子进程里，而 xv6 每个进程能打开的文件描述符总数只有 16 个 （见 defs.h 中的 NOFILE 和 proc.h 中的 struct file *ofile[NOFILE]; // Open files）

//一开始的思路是用non-block的方式来在只使用一个pipe的case下进行read，但是xv6并不支持。，然后回去看hint/manual。block发生的前提是write端口打开！
// int set_non_blocking(int fd) {
//     int flags = fcntl(fd, F_GETFL, 0);
//     if (flags == -1) {
//         perror("fcntl");
//         return -1;
//     }
//     flags |= O_NONBLOCK;
//     if (fcntl(fd, F_SETFL, flags) == -1) {
//         perror("fcntl");
//         return -1;
//     }

//     return 0;
// }


//=======================下面是只使用一组pipe的写法，输出一次prime： 2之后就block了===========================
// int main(){
// 	//局部变量声明
// 	int pl[2];//左管道
// 	int pr[2];//右管道
// 	int buf=0;
// 	//初始化
// 	pipe(pl);
// 	pipe(pr);
// 	for(int i=2;i<36;i++)
// 		write(pl[1],(int *)(&i),sizeof(int));//妙用&
	
// 	//每循环一次，输出min值并且将更新后的stream输出到pr
// 	while(1){
// 		printf("now getpid()==%d\n",getpid());
// 		if(read(pl[0],&buf,sizeof(int))==0)
// 			exit(0);
		
// 		int temp=buf;
// 		printf("prime :%d\n",buf);
// 		int temp2=0;
// 		while(1){
// 			close(pl[1]);//很关键
// 			temp2=read(pl[0],&buf,sizeof(int));//这里有一个易错点，如果pipe为空你对它进行读取，不会返回0/-1
// 			//而是会block！啥也不返回
// 			printf("temp2==%d\n",temp2);
// 			if(temp2!=sizeof(int))
// 				break;
// 			printf("buf==%d\n",buf);
// 			if(buf%temp!=0)
// 				write(pr[1],&buf,sizeof(int));
// 		}
// 		//printf("while terminated\n");
// 		if(fork()==0){
// 			//子进程：希望当前的pr指向的pipe由pl指向，同时pr须清空
// 			printf("now getpid()==%d\n",getpid());
// 			while(read(pr[0],&buf,sizeof(int))!=0)
// 				write(pl[1],&buf,sizeof(int));
// 		}
// 		else{
			
// 			wait(0);
// 			exit(0);
// 		}
// 	}
// }

// //==========================综上所述，由于block机制的存在仅仅使用两个管道是实现不了的，下面是multi-pipe实现=======================
// //第一次尝试（失败，目前尚未搞清楚原因，猜测与dup的行为有关）
// int main(){
// 	//局部变量声明
// 	int pl[2];//左管道
// 	int pr[2];//右管道
// 	int buf=0;
// 	//初始化
// 	pipe(pl);
// 	pipe(pr);
// 	close(pl[0]);//一开始漏了这一个，然后在第二层循环block了
// 	for(int i=2;i<36;i++)
// 		write(pr[1],(int *)(&i),sizeof(int));//妙用&
// 	int temp_fd=0;
// 	while(1){
// 		//每层循环都创建一组新的pipe，这里要注意创建新pipe和读取之前的pipe的顺序
// 		printf("===========pid:%d =============\n",getpid());
// 		//step1,读取前一个进程的pr,基于前面的分析可知，此时必须要关闭前一个进程的pr的write端口
// 		close(pr[1]);
// 		int temp=0;
// 		if(read(pr[0],&temp,sizeof(int))==0)
// 			exit(0);
		
// 		printf("prime :%d\n",temp);
// 		//close(pr[0]);//基于节约资源的原则
		
// 		temp_fd=dup(pr[0]);
		
// 		close(pr[0]);
// 		//step2,新建一个pr,把符合要求的数字传入pr,供下一个进程读取
// 		pipe(pr);
// 		while(read(temp_fd,&buf,sizeof(int))){
// 			//printf("pid:%d \n",getpid());
// 			if(buf%temp!=0){
// 				//printf("write once\n");
// 				write(pr[1],&buf,sizeof(int));
// 					printf("write:%d\n",buf);
// 			}
// 		}
// 		// while(read(pr[0],&buf,sizeof(int))){
			
// 		// 	printf("%d\n",buf);
// 		// }

// 		printf("loop finish\n");
// 		close(temp_fd);
// 		if(fork()==0){
// 			close(pr[1]);
// 			;
// 		}	
// 		else{
// 			wait(0);
// 			exit(0);
// 		}


// 	}


// }

//基于pipe复用第二次尝试------采用两组pipe
//（其实每个进程都使用了不同pipe，但是巧妙地运用了pl和pr的同名，增强了代码的复用性）
int main(){
	//局部变量声明
	int pl[2];//左管道
	int pr[2];//右管道
	int buf=0;
	//初始化
	pipe(pl);
	//pipe(pr);
	for(int i=2;i<36;i++)
		write(pl[1],(int *)(&i),sizeof(int));//妙用&
	while(1){
		//printf("===========pid:%d =============\n",getpid());
		//step1,读取pl,注意此时必须要关闭pl的write端口
		close(pl[1]);
		int temp=0;
		if(read(pl[0],&temp,sizeof(int))==0)
			exit(0);		
		printf("prime %d\n",temp);
		close(pr[0]);
		//step2,新建一个pr,把符合要求的数字传入pr,供下一个进程读取
		pipe(pr);
		while(read(pl[0],&buf,sizeof(int))){
			//printf("pid:%d \n",getpid());
			if(buf%temp!=0){
				write(pr[1],&buf,sizeof(int));
			}
		}
		close(pl[0]);//基于节约资源的原则,xv6文件描述符总数有限
		if(fork()==0){
			//根据父进程的pr，生成自己的pl============//这是否多此一举？//直接用一个pipe用来通信会不会有麻烦？
			pipe(pl);
			close(pr[1]);
			while(read(pr[0],&buf,sizeof(int)))
				write(pl[1],&buf,sizeof(int));
			close(pr[0]);
		}	
		else{
			close(pr[1]);
			close(pr[0]);
			wait(0);
			exit(0);
		}
	}
}










//========================一些其他做法=============================================
// void seive(int left_pipe[2]){

// 	close(left_pipe[1]);
// 	int temp=0;
// 	read(left_pipe[0],(void*)(&temp),sizeof(int));
// 	if(temp==-1){
// 		//在输入的时候就要有意识地设置哨兵了
// 		exit(0);
// 	}
// 	printf("prime %d\n",temp);
// 	int right_pipe[2];
// 	pipe(right_pipe);
// 	int pid=fork();
// 	if(pid==0){
// 		close(left_pipe[0]);
// 		close(right_pipe[1]);
// 		seive(right_pipe);
// 		exit(0);

// 	}

// 	else{
// 		close(right_pipe[0]);
// 		//父进程先向right_pipe write
// 		int temp_2=0;
// 		while(read(left_pipe[0],(void*)(&temp_2),sizeof(int))!=0&&temp_2!=-1)
// 			//!!这里加上temp_2!=-1非常重要，如果不加的话在输出完素数之后会进入死循环
// 		{
// 			if(temp_2%temp!=0)
// 			write(right_pipe[1],(void*)(&temp_2),sizeof(int));
// 		}
// 		//上面的写入应该不能保证原子性，开始写入之后，子进程的read就可以开始执行了
// 		temp=-1;//每一次都需要设置哨兵
// 		write(right_pipe[1],(void*)&temp_2,sizeof(int));
// 		wait(0);
// 		exit(0);
// 	}

// }

// int main(int argc,int *argv[]){

// 	int input_pipe[2];
// 	pipe(input_pipe);
	
// 	int pid=fork();
// 	if(pid==0){
// 		close(input_pipe[1]);
// 		seive(input_pipe);	

// 		exit(0);

// 	}

// 	else{
// 		//第一次的left_pipe由用户初始化
// 		for(int i=2;i<36;i++)
// 			write(input_pipe[1],(void*)(&i),sizeof(int));
// 		int temp=-1;//设置-1作为哨兵
// 		write(input_pipe[1],(void*)(&temp),sizeof(int));

// 		wait(0);
// 		exit(0);
// 	}//这里的wait只能等待一个子进程
// 	 return 0;
// }
