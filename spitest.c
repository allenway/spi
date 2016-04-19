#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "rha_spi_ioctl.h"
int main (int argc,char *argv[])  
{  
    int fd;
    int ret;
    unsigned short  data;
    unsigned int val;
    if(argc<3)
    {
        printf("Usage:\n");
        printf("write 16bit val:%s </dev/rha_spi> <val Hex>\n",argv[0]);
        return -1;
    }
    fd = open (argv[1], O_RDONLY);  
    if (fd <= 0)  
    {  
        printf ("open %s device error!\n",argv[1]);  
        return -1;  
    }
    else
        printf("open %s device successful!\n",argv[1]);
    sscanf(argv[2],"%x",&val);
    data = val;
    ret = ioctl(fd,RHA_SPI_CTL_W,&data);
    if(ret)
        printf ("ioctl write error!\n");  
    else
        printf ("ioctl write ok!\n");  
    close (fd);  
    return 0;  
}  


