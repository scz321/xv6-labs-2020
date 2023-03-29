#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

//=====================问题记录==================
// 1.参数分隔问题，对于argv参数数组的解析规则：这里argv数组的元素个数是不确定的，第一个元素是命令，后面的元素，
// 在这些元素没有包含'\n'之前的参数都是公共参数，
// 2.原来'\n'是一个char* 型的字符，而非字符串
// 3.你对参数来源的理解一开始存在偏差，你一开始天真地认为main函数中的char* argv[]参数就包含了公共参数部分
// 和私有参数部分,这显然是彻头彻尾的想当然。shell所做的也只是读取到xargs命令then进行exec，并不会帮助你进行
// 做其他工作.换言之，std input中的内容需要由你自己去读取。（本质上是读取fd=0的文件，这样就可以借助管道实现重定向)
//4.易错点。在子进程中调用exec时，你传入的整合之后的参数数组的argv_all[0]会被cmd覆盖！！！
//测试注意事项：以我下面的三个测试用例为例，它们都只会输出一行，尤其是要理解第三个测试用例也是只会输出一行。
//因为你用echo的方式是分别输出了\和n两个字符，但是如果不是echo的方式

//========测试用例===================
// echo 12 | xargs echo line
// echo "1\n2" | xargs echo line
//echo 1\n2 | xargs echo line
int main(int argc, char *argv[])
{
	//...先测试一下exec的用法。。。
	// exec(argv[1],argv+1);
	// exit(0);

	if (argc < 3)
	{
		printf("wrong argument numbers\n");
		exit(-1);
	}
	char *cmd = argv[1];
	// int temp=0;
	//  while(argv[temp]!=0){
	//  	printf("%s\n",argv[temp]);
	//  	temp++;
	//  }
	//printf("cmd:%s\n", cmd);

	// 关键部分,参数处理。
	// step1，获取每一行的“公共参数”
	int i = 2;
	char *argv_all[MAXARG];
	while(argv[i]!=0){
		argv_all[i-1]=argv[i];//注意这里argv_all的下标不能从0开始
		i++;
	}
	int psize = i-1; // 公共参数数量
	//printf("=====================\n");
	argv_all[psize + 1] = 0;
	// step2，获取每一行的“私有参数”，并同步地执行命令
	char argv_private[512]; // 通过read函数从fd=0处读取

	// printf("argv_private==%s\n",argv_private);
	char buf;
	int flag2 = 1; // flag用于控制外层循环的退出
	while (1)
	{
		// 每一层循环读出一个argv_private,执行一次exec
		int p = 0;
		while ((flag2 = read(0, &buf, sizeof(buf))) != 0 && buf != '\n')
		{
			*(argv_private + p) = buf;
			p++;
		}
		//printf("flag2==%d\n",flag2);
		if (flag2 == 0)
		{
			exit(0);
		}
		*(argv_private + p) = '\0';
		//printf("psize==%d\n",psize);
		argv_all[psize] = argv_private; // 当前状态的argv_public整合了私有参数
		if (fork() == 0)
		{
			//printf("%s\n", argv_all[1]);
			//printf("%s\n", argv_all[2]);
			// for (int i = 0; i < psize+1; i++)
			// {
			// 	printf("%s\n", argv_all[i]);
			// }
			//printf("%s\n", argv_all[3]);
			//printf("========exec start========\n");
			//printf("cmd==%s\n",cmd);
			argv_all[0]=cmd;
			//exec的第一个参数是path，emmmmmm
			//char*path=
			exec(argv_all[0], argv_all);
			//printf("========exec finish========\n");//你傻呀，exec不会返回，你这finish放这里有意义吗...
			exit(0);
		}
		else
		{
			wait(0);		
		}
		// //我的处理默认了argv_private中没有空格，或者说把空格作为参数的一部分而非分隔符，即以\n为唯一的参数分隔符
		// char buf[512];
		// char* p=buf;
		// while(argv_private[j]!='\n'){
		// 	*p=argv_private[j];
		// 	p++;
		// 	j++;
		// }
		// *p='\0';
		// argv_public[psize]=buf;//当前状态的argv_public整合了私有参数
		// if(fork()==0){
		// 	exec(cmd,argv_public);
		// 	exit(0);
		// }
		// else{
		// 	wait(0);
		// }
	}
	return 0;
}

// #define LENGTH 64

// //这个版本好像还是有bug

// //注意，转义字符占用y一个字节而不是2个char
// //还要注意，连续出现多个空格该如何处理
// //这个题目的难点其实在于弄清楚xargs的功能----flag参数的作用
// int main(int argc,char *argv[]){
// 	if(argc<2){
// 		printf("No enough argument!\n");
// 		exit(0);
// 	}
// 	char parse[MAXARG][LENGTH];
// 	char *argcs_per_line[LENGTH];//argcs_per_line用于记录从一行中解析出的所有参数

// 	char *cmd=argv[1];
// 	char *ch_read=0;//每次read读到的单个字符存储到ch_read中
// 					      //
// 	//外层循环终止条件：argv读取结果
// 	while(1){
// 		int count=0;//count用来记录某一行读取到的参数g的个数

// 	memset(parse,0,MAXARG*LENGTH);//初始化，每一次进入循环都要对记录参数的数组进行初始化

// 	int read_ret=-1;
// 	int cursor=0;//用来记录当前读到的位置
// 	int flag=0;//当read到的字符为空格时，如果flag=0，说明前一个字符>    不是空格，否则说明读到了连续的空格

// 	while((read_ret=read(0,ch_read,sizeof(char)))){
// 		//g根据读取结果是否是空格j进行分类讨论
// 		if(*ch_read==' '){
// 			if(flag==0){
// 				cursor=0;
// 				flag=1;

// 			}
// 			else{
// 				;
// 			}
// 		}
// 		else if(*ch_read=='\n')
// 			break;
// 		else{
// 			parse[count][cursor]=*ch_read;
// 			cursor++;
// 		}

// 	}
// 	if (read_ret <= 0) {
// 			break;
// 		}
// 		for (int i=0; i<MAXARG-1; i++) {
// 		argcs_per_line[i] = parse[i];
// 		}
// 		argcs_per_line[MAXARG-1] = 0;
// 		if (fork() == 0) {
// 			exec(cmd, argcs_per_line);
// 			exit(0);
// 		} else {
// 			wait((int *) 0);
// 		}
// 	}
// 	return 0;
// }
