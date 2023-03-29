#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

//==================问题整理=================
/*
1读取de.name只对文件夹有效，如果fd指向文件，然后执行read(fd, &de, sizeof(de))，得到的de.name会是奇怪的东西。
所以我借鉴ls中的fmtname函数，注意这里必须对fmt函数的细节做修改，不然使用strcmp进行比较时会出错
2似乎对文件以struct de的形式进行读取还会产生未知原因的block（console后面的文件无法被读取），所以本着安全第一的原则，
把read(fd, &de, sizeof(de))完全放在dir case中最安全
*/


char *
fmtname(char *path)//注意不能直接copy ls中的fmtname文件，否则会出错。
{
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), 0, DIRSIZ - strlen(p));//在此处做改动
  return buf;
}

void find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  if ((fd = open(path, 0)) < 0)
  {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0)
  { // 这里可以使用read函数来代替吗？类似于read(fd, &de, sizeof(de))
    return;
  }
  //printf("path==%s,st.type==%d\n",path,st.type);
  // printf("type of %s: %d\n",path,st.type);
  // if(st.type==T_DEVICE)
  //   {
  //     goto lable;
  //     //close(1);//但是关闭write端口似乎起不到效果？
  //   }
  //dev文件需要预先处理，否则后面的read会block？？ 
  //!因为这里的console文件和其他文件有着本质上的区别！难道是因为它的write端口是开的?
  //同时也要能够理解，read(fd, &de, sizeof(de)) != sizeof(de)和之前那种read(fd, &buf, sizeof(int))没有本质区别
    

  // if (read(fd, &de, sizeof(de)) != sizeof(de))
  //   exit(1);
  switch (st.type)
  {
  case T_FILE:
   // printf("=============T_FILE start==========\n\n");
    //printf("de.name==%s\n", de.name);
    //printf("fmtname(path)==%s\n",fmtname(path));
    //printf("target=%s\n", target);
    //int temp = strcmp(target,fmtname(path)); // 难道是大小写有问题？不知道为什么会产生32的差异
    //printf("temp==%d\n", temp);
    if (strcmp(target,fmtname(path)) == 0) // 不能使用==，而应该使用strcmp
      printf("%s\n", path);
   // printf("=============T_FILE finish==========\n\n");
    break;

  case T_DIR:
    //printf("=============T_DIR start==========\n\n");
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/'; // 优先级结合性了解一下
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      if ((de.inum == 0) || (strcmp(de.name, ".") == 0) || (strcmp(de.name, "..") == 0))
      {
        continue;
      }
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      
     // printf("de.name:%s\n", de.name);
      //printf("buf:%s\n", buf);
      find(buf, target);
     //printf("=============T_DIR finish==========\n\n");
    }
    break;
  }
  close(fd); // 有始有终
}

int main(int argc, char *argv[])
{

 // printf("start\n");
  if (argc != 3)
  {
    printf("wrong argument numbers!!!\n");
    exit(-1);
  }

  find(argv[1], argv[2]);
  exit(0);
}
