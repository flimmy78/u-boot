#include <common.h>
#include <command.h>
#include <config.h>

#if   defined(CONFIG_GODNET)
#include <godnet_vo.h>

#define VO_ADDRESS1 0x94000000
#define VO_ADDRESS2 0x95000000

#define VO_DEV0	VOU_DEV_DHD0
#define VO_LAY0	VOU_DEV_DHD0
#define VO_DEV1	VOU_DEV_DSD0
#define VO_LAY1	VOU_DEV_DSD0

#elif defined(CONFIG_GODARM)
#include <godarm_vo.h>

#define VO_ADDRESS1 0x87E00000
#define VO_ADDRESS2 0x87F00000

#define VO_DEV0	VOU_DEV_DHD0
#define VO_LAY0	VOU_DEV_DHD0
#define VO_DEV1	VOU_DEV_DSD1
#define VO_LAY1	VOU_DEV_DSD1

#elif defined(CONFIG_HI3520D)
#include <hi3520d_vo.h>

#define VO_ADDRESS1 0x87E00000
#define VO_ADDRESS2 0x87F00000

#define VO_DEV0	VOU_DEV_DHD0
#define VO_LAY0	VOU_DEV_DHD0
#define VO_DEV1	VOU_DEV_DSD1
#define VO_LAY1	VOU_DEV_DSD1

#elif   defined(CONFIG_HI3535)
#include <hi3535_vo.h>

#define VO_ADDRESS1 0x94000000
#define VO_ADDRESS2 0x95000000

#define VO_DEV0	VOU_DEV_DHD0
#define VO_LAY0	VOU_DEV_DHD0
#define VO_DEV1	VOU_DEV_DSD0
#define VO_LAY1	VOU_DEV_DSD0

#elif   defined(CONFIG_HI3536)
#include <hi3536_vo.h>

#define VO_ADDRESS1 0x48000000
#define VO_ADDRESS2 0x49000000

#define VO_DEV0	VOU_DEV_DHD0
#define VO_LAY0	VOU_DEV_DHD0
#define VO_DEV1	VOU_DEV_DSD0
#define VO_LAY1	VOU_DEV_DSD0

#elif   defined(CONFIG_HI3521A)
#include <hi3521a_vo.h>

#define VO_ADDRESS1 0x88000000
#define VO_ADDRESS2 0x89000000

#define VO_DEV0	VOU_DEV_DHD1
#define VO_LAY0	VOU_DEV_DHD1
#define VO_DEV1	VOU_DEV_DSD0
#define VO_LAY1	VOU_DEV_DSD0

#elif   defined(CONFIG_HI3531A)
#include <hi3531a_vo.h>

#define VO_ADDRESS1 0x48000000
#define VO_ADDRESS2 0x49000000

#define VO_DEV0	VOU_DEV_DHD1
#define VO_LAY0	VOU_DEV_DHD1
#define VO_DEV1	VOU_DEV_DSD0
#define VO_LAY1	VOU_DEV_DSD0

#else
#error "Platform Un-supported!!"

#endif

extern unsigned long ImgWidth, ImgHeight;
extern int load_jpeg(unsigned long des_buf, unsigned long src_addr, unsigned long src_size);
extern int jpeg_decode(void);

extern int set_vobg(unsigned int dev, unsigned int rgb);
extern int start_vo(unsigned int dev, unsigned int type, unsigned int sync);
extern int stop_vo(unsigned int dev);
extern int start_gx(unsigned int layer, unsigned addr, unsigned int strd, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
extern int stop_gx(unsigned int layer);
extern int hdmi_display(unsigned int HdFmt, unsigned int input, unsigned int output);

extern int check_vo_support(unsigned int dev, unsigned int type, unsigned int sync);

typedef struct {
	unsigned long Jpeg_Addr;
	unsigned long Jpeg_Size;
	unsigned long Video_Buf;
	unsigned int  Des_Lay;
	unsigned int  Des_Dev;
	unsigned int  Des_Dev_Type;
	unsigned int  Des_Dev_Sync;
	unsigned long JWidth;
	unsigned long JHeight;
} LogoParam_Struct;

LogoParam_Struct ParamBox[] = {
    {
	.Jpeg_Addr = 0x84800000,
	.Jpeg_Size = 0x00040000,
	.Video_Buf = VO_ADDRESS1,
	.Des_Lay   = VO_LAY0,
	.Des_Dev   = VO_DEV0,
	.Des_Dev_Type = VO_INTF_VGA | VO_INTF_HDMI,
	.Des_Dev_Sync = 13,
	.JWidth  = 0,
	.JHeight = 0,
	}, {
	.Jpeg_Addr = 0x84900000,
	.Jpeg_Size = 0x00040000,
	.Video_Buf = VO_ADDRESS2,
	.Des_Lay   = VO_LAY1,
	.Des_Dev   = VO_DEV1,
	.Des_Dev_Type = VO_INTF_CVBS,
	.Des_Dev_Sync = VO_OUTPUT_PAL,
	.JWidth  = 0,
	.JHeight = 0,
	}
};

int VO_CONV_TBL[][3] = {
    {VO_OUTPUT_1080P24, 1920, 1080},
    {VO_OUTPUT_1080P25, 1920, 1080},
    {VO_OUTPUT_1080P30, 1920, 1080},

    {VO_OUTPUT_720P50,  1280, 720},
    {VO_OUTPUT_720P60,  1280, 720},
    {VO_OUTPUT_1080I50, 1920, 1080},
    {VO_OUTPUT_1080I60, 1920, 1080},
    {VO_OUTPUT_1080P50, 1920, 1080},
    {VO_OUTPUT_1080P60, 1920, 1080},

    {VO_OUTPUT_576P50, 720, 576},
    {VO_OUTPUT_480P60, 720, 480},

    {VO_OUTPUT_800x600_60,    800, 600},
    {VO_OUTPUT_1024x768_60,  1024, 768},
    {VO_OUTPUT_1280x1024_60, 1280, 1024},
    {VO_OUTPUT_1366x768_60,  1366, 768},
    {VO_OUTPUT_1440x900_60,  1440, 900},
    {VO_OUTPUT_1280x800_60,  1280, 800},
};

int do_logo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   int i;
   char *tmpStr;
   ulong LogoAddr[sizeof(ParamBox)/sizeof(ParamBox[0])];
   ulong VoutAddr[sizeof(ParamBox)/sizeof(ParamBox[0])];

#if !defined(CONFIG_HI3531A)
   enable_mmu();
   dcache_enable();
   printf("enable_mmu_accelerate\n");
#endif

   memset(&LogoAddr, 0, sizeof(LogoAddr));
   memset(&VoutAddr, 0, sizeof(VoutAddr));

   tmpStr = getenv ("logo_vga");
   if (tmpStr) {
      LogoAddr[0] = simple_strtoul(tmpStr, NULL, 16);
   }
   tmpStr = getenv ("logo_pal");
   if (tmpStr) {
      LogoAddr[1] = simple_strtoul(tmpStr, NULL, 16);
   }
   tmpStr = getenv ("vout_vga");
   if (tmpStr) {
      VoutAddr[0] = simple_strtoul(tmpStr, NULL, 16);
   }
   tmpStr = getenv ("vout_pal");
   if (tmpStr) {
      VoutAddr[1] = simple_strtoul(tmpStr, NULL, 16);
   }

   for(i = 0; i < sizeof(ParamBox)/sizeof(ParamBox[0]); i ++) {
      LogoParam_Struct * pLogo = &ParamBox[i];
      if(!load_jpeg(VoutAddr[i] ? VoutAddr[i] : pLogo->Video_Buf,
               LogoAddr[i] ? LogoAddr[i] : pLogo->Jpeg_Addr,
               pLogo->Jpeg_Size)
            && !jpeg_decode()) {
         pLogo->JWidth  = ImgWidth;
         pLogo->JHeight = ImgHeight;
      }
      else {
         pLogo->JWidth  = 0;
         pLogo->JHeight = 0;
      }
   }

   for(i = 0; i < sizeof(ParamBox)/sizeof(ParamBox[0]); i ++) {
      int ii;
      LogoParam_Struct * pLogo = &ParamBox[i];

      if(!pLogo->JWidth || !pLogo->JHeight) continue;
      if(pLogo->JWidth > PIC_MAX_WIDTH
         || (pLogo->JWidth & 0x1)
         || pLogo->JWidth < PIC_MIN_LENTH
         || pLogo->JHeight > PIC_MAX_HEIGHT
         || (pLogo->JHeight & 0x1)
         || pLogo->JHeight < PIC_MIN_LENTH) {
            continue;
      }

      for(ii = 0;
         pLogo->Des_Dev_Type != VO_INTF_CVBS
            && ii < sizeof(VO_CONV_TBL)/sizeof(VO_CONV_TBL[0]);
         ii ++) {
            if(pLogo->JWidth == VO_CONV_TBL[ii][1]
               && pLogo->JHeight == VO_CONV_TBL[ii][2]) {
                  pLogo->Des_Dev_Sync = VO_CONV_TBL[ii][0];
            }
      }

      if (check_vo_support(pLogo->Des_Dev, pLogo->Des_Dev_Type, pLogo->Des_Dev_Sync)) {
            printf("Unsupport parameter!\n");
            continue;
      }

      start_vo(pLogo->Des_Dev, pLogo->Des_Dev_Type, pLogo->Des_Dev_Sync);
      start_gx(pLogo->Des_Lay, VoutAddr[i] ? VoutAddr[i] : pLogo->Video_Buf, 
               pLogo->JWidth*2,
               0, 0,
               pLogo->JWidth, pLogo->JHeight);
      if (pLogo->Des_Dev_Type & VO_INTF_HDMI) {
         hdmi_display(pLogo->Des_Dev_Sync, 2, 2);
      }
   }

#if !defined(CONFIG_HI3531A)
   dcache_disable();
   stop_mmu();
   printf("disable_mmu_accelerate\n");
#endif

   return 0;
}

U_BOOT_CMD(
	logo,    CFG_MAXARGS,	1,  do_logo,
	"logo - decode logo picture to show.\n",
	"\t- jpeg picture addr must be set.\n"
	);


