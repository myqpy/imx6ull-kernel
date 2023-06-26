#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
*argc:应用程序参数个数
*argv[]:具体的参数内容，字符串行驶
*./chrdevbaseAPP.c <filename>
*/
int main(int argc, char* argv[])
{
    int ret = 0;
    int fd = 0;
    char *filename;
    char readbuf[100],writebuf[100];

    filename = argv[1];

    // int open(const char *pathname, int flags);
    fd = open(filename, O_RDWR);
    if(fd < 0) 
    {
        printf("Can't Open file %s\r\n",filename);
        return -1;
    }

// ssize_t read(int fd, void *buf, size_t count);
    ret = read(fd,readbuf,50);
    if(ret < 0)
    {
        printf("read file %s failed \r\n", filename);
        return -1;
    }
    else{}

// ssize_t write(int fd, const void *buf, size_t count);
    ret = write(fd,writebuf,50);
    if(ret < 0)
    {
        printf("write file %s failed \r\n", filename);
        return -1;
    }
    else{}

    ret = close(fd);
    if(ret < 0)
    {
        printf("close file %s failed \r\n", filename);
        return -1;
    }

    return 0;

}