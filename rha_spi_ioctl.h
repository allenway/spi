/*
 * *******************************************************************************
 * *   FileName    :    rha_i2c_ioctl.h
 * *   Description :    i2c ioctl

 * *   Author      :    2016-2-29 10:33   by   liulu
 * ********************************************************************************
 * */
#ifndef __RHA_SPI_IOCTL_H_
#define __RHA_SPI_IOCTL_H_
#include<linux/ioctl.h>
#define RHA_SPI_TYPE 'L'
#define RHA_SPI_SIZE sizeof(unsigned short)
#define RHA_SPI_CMD_W   1
#define RHA_SPI_CTL_W _IOWR(RHA_SPI_TYPE,RHA_SPI_CMD_W,RHA_SPI_SIZE)
#endif
