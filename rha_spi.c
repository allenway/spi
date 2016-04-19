/*
 * *******************************************************************************
 * *   FileName    :    rha_spi.c
 * *   Description :
 * *   Author      :    2016-2-29 18:34   by   liulu
 * ********************************************************************************
 * */
#include <linux/module.h>     /* 引入与模块相关的宏 */
#include <linux/init.h>        /* 引入module_init() module_exit()函数 */
#include <linux/moduleparam.h> /* 引入module_param() */
#include <linux/workqueue.h>
#include <asm/io.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/fs.h> 
#include <asm/uaccess.h>
#include "rha_spi.h"
#include "rha_spi_ioctl.h"
//#define __DEBUG
static struct file_operations spi_fops; 
static struct rha_spi rha_spi_dev[] = {
    {
        .clk = {
            .mux_phy_addr = 0x20E00F8,
            .pad_phy_addr = 0x20E040C,
            .gpio_phy_addr = 0x20A0000,
            .gpio_phy_size = 0x20,
            .gpio_dr_offset = 0x0,
            .gpio_gdr_offset = 0x4,
            .gpio_psr_offset = 0x8,
            .gpio_icr1_offset = 0xc,
            .gpio_icr2_offset = 0x10,
            .gpio_imr_offset = 0x14,
            .gpio_isr_offset = 0x18,
            .gpio_edge_sel_offset = 0x1c,
            .gpio_nr = 23,
            .mux = NULL,
            .pad = NULL,
            .gpio_base = NULL,
        },
        .sdi = {
            .mux_phy_addr = 0x20E00FC,
            .pad_phy_addr = 0x20E0410,
            .gpio_phy_addr = 0x20A0000,
            .gpio_phy_size = 0x20,
            .gpio_dr_offset = 0x0,
            .gpio_gdr_offset = 0x4,
            .gpio_psr_offset = 0x8,
            .gpio_icr1_offset = 0xc,
            .gpio_icr2_offset = 0x10,
            .gpio_imr_offset = 0x14,
            .gpio_isr_offset = 0x18,
            .gpio_edge_sel_offset = 0x1c,
            .gpio_nr = 24,
            .mux = NULL,
            .pad = NULL,
            .gpio_base = NULL,
        },
        .le = {
            .mux_phy_addr = 0x20E01F0,
            .pad_phy_addr = 0x20E0504,
            .gpio_phy_addr = 0x209C000,
            .gpio_phy_size = 0x20,
            .gpio_dr_offset = 0x0,
            .gpio_gdr_offset = 0x4,
            .gpio_psr_offset = 0x8,
            .gpio_icr1_offset = 0xc,
            .gpio_icr2_offset = 0x10,
            .gpio_imr_offset = 0x14,
            .gpio_isr_offset = 0x18,
            .gpio_edge_sel_offset = 0x1c,
            .gpio_nr = 30,
            .mux = NULL,
            .pad = NULL,
            .gpio_base = NULL,
        },
        .oe = {
            .mux_phy_addr = 0x20E0090,
            .pad_phy_addr = 0x20E03A4,
            .gpio_phy_addr = 0x20A4000,
            .gpio_phy_size = 0x20,
            .gpio_dr_offset = 0x0,
            .gpio_gdr_offset = 0x4,
            .gpio_psr_offset = 0x8,
            .gpio_icr1_offset = 0xc,
            .gpio_icr2_offset = 0x10,
            .gpio_imr_offset = 0x14,
            .gpio_isr_offset = 0x18,
            .gpio_edge_sel_offset = 0x1c,
            .gpio_nr = 16,
            .mux = NULL,
            .pad = NULL,
            .gpio_base = NULL,
        },
        .misc = {
            .minor = MISC_DYNAMIC_MINOR,
            .name = "rha_spi",
            .fops = &spi_fops,
        }
    },
#if 0
    {
        .scl = {
            .mux = NULL,
            .pad = NULL,
            .gpio_base = NULL,
        },
        .sda = {
            .mux = NULL,
            .pad = NULL,
            .gpio_base = NULL,
        },
        .misc = {
            .minor = MISC_DYNAMIC_MINOR,
            .name = "rha_spi_1",
            .fops = &spi_fops,
        }
    },
#endif
};
#define RHA_spi_DEV_NUM  (sizeof(rha_spi_dev)/(sizeof(struct rha_spi)))

static int  spi_write_val(struct rha_spi *spi,unsigned short spi_data);
//spi misc
static long spi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)  
{  
    struct rha_spi *spi = filp->private_data;
    unsigned short spi_data;
    int ret = 0;
#ifdef __DEBUG
    printk ("spi_ioctl:%s\n",spi->misc.name);
#endif
    switch(_IOC_NR(cmd)){
        case RHA_SPI_CMD_W:
            if(copy_from_user((void *)&spi_data,(void *)arg,RHA_SPI_SIZE))
            {
                ret = -1;
                break;
            }
            ret = spi_write_val(spi,spi_data);
            break;
        default:
            ret = -1;
    }
    return ret;  
}  

static int spi_open(struct inode *inode, struct file *filp)  
{  
    int minor = iminor(inode);
    int i;
    for(i=0;i<RHA_spi_DEV_NUM;i++)
    {
        if(minor==rha_spi_dev[i].misc.minor)
        {
            filp->private_data = &rha_spi_dev[i];
            break;
        }
    }
    if(i<RHA_spi_DEV_NUM)
    {
#ifdef __DEBUG
        printk("spi_open %s ok\n",rha_spi_dev[i].misc.name);
#endif
        return 0;
    }
    else
    {
#ifdef __DEBUG
        printk("spi_open fail\n");
#endif
        return -1;
    }
}  
static int spi_release(struct inode *inode, struct file *filp)  
{  
#ifdef __DEBUG
    printk ("spi_release\n");  
#endif
    return 0;  
}  

static struct file_operations spi_fops =  
{  
    .owner   = THIS_MODULE,  
    .unlocked_ioctl = spi_ioctl,
    .open    = spi_open,  
    .release = spi_release  
};  
//mode 1 output ,0 input
static inline void gpio_set_mode(struct rha_gpio *gpio,int mode)
{
    unsigned int val;
    val = ioread32(gpio->gpio_base+gpio->gpio_gdr_offset);
    if(mode)  //output
        val |= (0x1<<gpio->gpio_nr); //set output
    else
        val &= ~(0x1<<gpio->gpio_nr); //set input
    iowrite32(val,gpio->gpio_base+gpio->gpio_gdr_offset);

};
static inline void gpio_set_val(struct rha_gpio *gpio,int n)
{
    unsigned int val;
    val = ioread32(gpio->gpio_base+gpio->gpio_dr_offset);
    if(n)  //output 1
        val |= (0x1<<gpio->gpio_nr); //output 1
    else
        val &= ~(0x1<<gpio->gpio_nr); //output 0
    iowrite32(val,gpio->gpio_base+gpio->gpio_dr_offset);
};
static inline int gpio_get_val(struct rha_gpio *gpio)
{
    unsigned int val;
    val = ioread32(gpio->gpio_base+gpio->gpio_dr_offset);
    return (val & (0x1<<gpio->gpio_nr))?1:0;
};
//unit us
#define T_CLK    50 
#define T_SDI    50
#define T_LE     100
static void spi_write(struct rha_spi *spi,unsigned short n)
{
    int i;
    //write 16bit data
    //gpio_set_mode(&spi->clk,1);
    //gpio_set_mode(&spi->sdi,1);
    gpio_set_val(&spi->le,0);
    ndelay(T_LE);
    for(i=0;i<16;i++)
    {
        gpio_set_val(&spi->sdi,0x1&(n>>i));
        ndelay(T_SDI);
        
        gpio_set_val(&spi->clk,1);
        ndelay(T_CLK);
        
        gpio_set_val(&spi->clk,0);
        ndelay(T_CLK);
    }
    gpio_set_val(&spi->le,1);
    ndelay(T_LE);
    gpio_set_val(&spi->le,0);
    ndelay(T_LE);
    gpio_set_val(&spi->clk,0);
    gpio_set_val(&spi->sdi,0);
    ndelay(T_LE);
};
//
static int  spi_write_val(struct rha_spi *spi,unsigned short spi_data)
{
#ifdef __DEBUG
    printk ("%s val:%x \n",__func__,spi_data);
#endif 
    mutex_lock(&spi->mutex);
    spi_write(spi,spi_data);
    mutex_unlock(&spi->mutex);
    return 0;
}

static int __init rha_gpio_init(struct rha_gpio *gpio)
{
    volatile unsigned int val;
    printk(KERN_INFO"%s entery\n",__func__);
    //1.初始化HW
    //ioremap
    gpio->mux = ioremap(gpio->mux_phy_addr,4);
    if(gpio->mux==NULL)
    {
        printk(KERN_INFO"%s ioremap for gpio->mux_phy_addr failed",__func__);
        goto GPIO_ERR0;
    }
    gpio->pad = ioremap(gpio->pad_phy_addr,4);
    if(gpio->pad==NULL)
    {
        printk(KERN_INFO"%s ioremap for gpio->pad_phy_addr failed ",__func__);
        goto GPIO_ERR1;
    }
    gpio->gpio_base = ioremap(gpio->gpio_phy_addr,gpio->gpio_phy_size);
    if(gpio->gpio_base==NULL)
    {
        printk(KERN_INFO"%s ioremap for gpio->gpio_phy_addr failed",__func__);
        goto GPIO_ERR2;
    }

    //初始化iomux as gpio mode
    val = ioread32(gpio->mux);
    val = (val & (~0x17)) | 0x5;  //set gpio mode
    iowrite32(val,gpio->mux);
    val = ioread32(gpio->pad);
    val = (val & (~0x1f8f9)) | 0x30b0;  //pull down
    iowrite32(val,gpio->pad);

    //初始化gpio
    val = ioread32(gpio->gpio_base+gpio->gpio_dr_offset);
    val &= ~(0x1<<gpio->gpio_nr); //output 0
    iowrite32(val,gpio->gpio_base+gpio->gpio_dr_offset);
    
    val = ioread32(gpio->gpio_base+gpio->gpio_gdr_offset);
    val |= (0x1<<gpio->gpio_nr); //set output
    iowrite32(val,gpio->gpio_base+gpio->gpio_gdr_offset);

    printk(KERN_INFO"%s ok\n",__func__);
    return 0;
GPIO_ERR2:
    iounmap(gpio->pad);
GPIO_ERR1:
    iounmap(gpio->mux);
GPIO_ERR0:
    printk(KERN_INFO"%s failed\n",__func__);
    return -1;
}
static void __exit rha_gpio_uninit(struct rha_gpio *gpio)
{
    //4.反初始化HW
    iounmap(gpio->mux);
    iounmap(gpio->pad);
    iounmap(gpio->gpio_base);
    return;
}
static int __init rha_spi_dev_init(struct rha_spi *spi)
{
    int ret;
    //gpio init
    ret = rha_gpio_init(&spi->clk);
    if(ret)
    {
        printk ("rha_gpio_init clk fail\n");
        goto CLK_ERR;
    }
    else
        printk ("rha_gpio_init clk ok\n");
    ret = rha_gpio_init(&spi->sdi);
    if(ret)
    {
        printk ("rha_gpio_init sdi fail\n");
        goto SDI_ERR;
    }
    else
        printk ("rha_gpio_init sdi ok\n");

    ret = rha_gpio_init(&spi->le);
    if(ret)
    {
        printk ("rha_gpio_init le fail\n");
        goto LE_ERR;
    }
    else
        printk ("rha_gpio_init le ok\n");
    ret = rha_gpio_init(&spi->oe);
    if(ret)
    {
        printk ("rha_gpio_init oe fail\n");
        goto OE_ERR;
    }
    else
        printk ("rha_gpio_init oe ok\n");
    //set default val
    gpio_set_val(&spi->clk,0);
    gpio_set_val(&spi->sdi,0);
    gpio_set_val(&spi->le,0);
    gpio_set_val(&spi->oe,0);
    //register misc dev
    ret = misc_register(&spi->misc);  
    if(ret)
    {
        printk ("misc_register fail\n");
        goto MISC_ERR;
    }
    else
        printk ("misc_register ok\n");
    mutex_init(&spi->mutex);
    return 0;
MISC_ERR:
    rha_gpio_uninit(&spi->oe);
OE_ERR:
    rha_gpio_uninit(&spi->le);
LE_ERR:
    rha_gpio_uninit(&spi->sdi);
SDI_ERR:
    rha_gpio_uninit(&spi->clk);
CLK_ERR:
    return ret;
}
static void __exit rha_spi_dev_uninit(struct rha_spi *spi)
{
    mutex_destroy(&spi->mutex);
    misc_deregister(&spi->misc);
    rha_gpio_uninit(&spi->clk);
    rha_gpio_uninit(&spi->sdi);
}
static int __init rha_spi_init(void)
{
    int ret = -1;
    int i;
    printk(KERN_INFO"%s entery\n",__func__);
    for(i=0;i<RHA_spi_DEV_NUM;i++)
    {
        ret = rha_spi_dev_init(&rha_spi_dev[i]);
        if(ret)
        {
            printk ("rha_spi_init failed: name %s\n",rha_spi_dev[i].misc.name);
            while(i){
                i--;
                rha_spi_dev_uninit(&rha_spi_dev[i]);
            }
            break;
        }
        else
            printk ("rha_spi_init ok: name %s\n",rha_spi_dev[i].misc.name);
    }
    printk(KERN_INFO"%s %s\n",__func__,ret==0?"ok":"fail");
    return ret;
}

static void __exit rha_spi_exit(void)
{
    int i;
    printk(KERN_INFO"%s entery\n",__func__);
    for(i=0;i<RHA_spi_DEV_NUM;i++)
                rha_spi_dev_uninit(&rha_spi_dev[i]);
    printk(KERN_INFO"%s ok\n",__func__);
}
module_init(rha_spi_init);
module_exit(rha_spi_exit);
MODULE_AUTHOR("RHA,Liu Lu");
MODULE_LICENSE("GPL"); 
MODULE_DESCRIPTION("spi driver for RHA PIS");

