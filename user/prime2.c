#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//目前来看，这种思路和我的思路的主要区别在于，我企图只用一个pipe来让所有进程公用
void seive(int left_pipe[2]){

	int temp=0;
	read(left_pipe[0],(void*)(&temp),sizeof(int));
	if(temp==-1){
		//在输入的时候就要有意识地设置哨兵了
		exit(0);
	}
	printf("prime: %d\n",temp);
	int right_pipe[2];
	pipe(right_pipe);
	int pid=fork();
	if(pid==0){

		seive(right_pipe);

	}
	else{
		//父进程先向right_pipe write
		int temp_2=0;
		while(read(left_pipe[0],(void*)(&temp_2),sizeof(int))!=0)
		{
			if(temp_2%temp!=0)
			write(right_pipe[1],(void*)(&temp_2),sizeof(int));
		}
		//上面的写入应该不能保证原子性，开始写入之后，子进程的read就可以开始执行了
		wait(0);
		exit(0);
	}

}




int main(int argc,int *argv[]){

	int input_pipe[2];
	pipe(input_pipe);

	int pid=fork();
	if(pid==0){
		seive(input_pipe);	


	}

	else{
		//第一次的left_pipe由用户初始化
		for(int i=2;i<36;i++)
			write(input_pipe[1],(void*)(&i),sizeof(int));
		int temp=-1;//设置-1作为哨兵
		write(input_pipe[1],(void*)(&temp),sizeof(int));

		wait(0);
		exit(0);
	}//这里的wait只能等待一个子进程
	 return 0;
}
