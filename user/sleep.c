#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{


  //what if argc>1??
  if(argc<=1){
	printf("please pass an argument,OK?\n");
	exit(1);
  }
  else{
	//参数默认是字符型，所以需要转换
	//这样就是经典错误了int time=argv[1];
	
	//暂时不支持错误输入。比如输入字母...。因为主要目的在于熟悉系统调用及其过程
  	
	sleep(atoi(argv[0]));
	
  exit(0);
}
}
