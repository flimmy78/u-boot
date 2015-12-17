#include <common.h>
#include <command.h>
#include <malloc.h>
//#include <devices.h>
#include <version.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/sizes.h>
#include <config.h>

unsigned long jpeg_size=0;

typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16;

extern UINT32  LoadJpegFile(void *pImg);
extern unsigned long ImgWidth, ImgHeight;

const char hilogo_magic_str[] = "HISILICON LOGO MAGIC";

#define JPEG_BUFFER_SIZE (4*1024*1024)
unsigned char hilogo[JPEG_BUFFER_SIZE];
unsigned long VIDEO_DATA_BASE=0;

extern void dcache_enable(void);
extern void dcache_disable(void);
extern int dcache_status(void);
extern void enable_mmu(void);
extern void stop_mmu(void);

int jpeg_decode(void)
{
    //enable_mmu();

#ifndef CONFIG_HI3531A
    //dcache_enable();
#endif
    //printf("mmu_enable\n");

    LoadJpegFile((void *)VIDEO_DATA_BASE);
    
#ifndef CONFIG_HI3531A
    //dcache_disable();
#endif
    //stop_mmu();

    return 0;
}

int load_jpeg(unsigned long des_buf, unsigned long src_addr, unsigned long src_size)
{
    if(!src_addr || !src_size || !des_buf) {
       printf("PARAMETER CANNOT BE ZERO!\n");
       return -1;
    }

    VIDEO_DATA_BASE = des_buf;
    jpeg_size = src_size;
    printf("<<addr=%#x, size=%#x, vobuf=%#x>>\n", src_addr, src_size, VIDEO_DATA_BASE);

    memset(hilogo, 0x0, sizeof(hilogo));
    memcpy(hilogo, (char*)src_addr, src_size);

    if (hilogo[0] != 0xFF || hilogo[1] != 0xD8) {
        printf("addr:%#x,size:%d,logoaddr:%#x,:%2x,%2x\n",
                  hilogo,
                  src_size,
                  src_addr,
                  hilogo[0],
                  hilogo[1]);
        return -1;
    }

    return 0;
}
