#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>

#define BUF_LEN PIPE_BUF

int fifofd = -1;
char *fifoname;
char *border = "****************************************";

void process_data(char *buf, int len)
{
    if(len < 20){
	    return;
    }
    if (len > 0){
        write(2, buf, len);
        
    }
    return;
}

int main()
{
    fifoname = getenv("FIFONIDS");
    //fifoname = "/home/stray/桌面/nids-based/fifo";
    printf("Open fifo...\n");
    if (access(fifoname, F_OK) != -1)
    {
        printf("fifo exists:%s\n", fifoname);
    }
    else
    {
        perror("fifo not exist. Exiting...\n");
        exit(1);
    }
    if ((fifofd = open(fifoname, O_RDONLY)) < 0)
    {
        perror("Open fifo failed!\n");
        exit(1);
    }
    printf("Open fifo success\n");

    char *buf = (char *)malloc(PIPE_BUF);
    int len;
    while ((len = read(fifofd, buf, PIPE_BUF)) > 0)
    {
	if (len < 20) continue;
	printf("\n%s",border);
        printf("recv data, len:%d", len);
	printf("%s\n",border);
        process_data(buf, len);
    }
}
