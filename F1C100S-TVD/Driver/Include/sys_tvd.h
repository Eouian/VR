/*
*********************************************************************************************************

* File    : tvd.h
* Version : v1.0
* Update  : date                auther         ver     notes
*           2016-01-14    charles cheng   3.0     upgrade the driver
*********************************************************************************************************
*/

#ifndef _TVD_H_
#define _TVD_H_
#define __s32 int
#define __u32 unsigned int 
#define EPDK_OK 1

extern int TVD_OUT_W;
extern int TVD_OUT_H;


typedef enum  e_TVD_SEL_SOURCE
{
    TVD_SOURCE_CVBS=1,
    TVD_SOURCE_YPbPr,

    TVD_SEL_SOURCE_MAX

}__drv_TVD_sel_source;

typedef enum  e_TVD_PAL_NTSC
{
	TVD_SOURCE_NTSC=1,
	TVD_SOURCE_PAL,
	TVD_SOURCE_PAL_M,
	TVD_SOURCE_PAL_N,
	TVD_SOURCE_SECAM,
	TVD_SYSTEM_FORMAT_MAX

}__drv_TVD_system;

typedef enum
{
    TVD_PL_YUV420                       =   0,  // ????????a??
    TVD_MB_YUV420                       =   1,  // ?????????
    TVD_PL_YUV422                       =   2,  // ????????a??
}TVD_FMT_T;


typedef enum
{
    TVD_NTSC=1,
    TVD_PAL,
}tvd_mode_t;

typedef enum
{
    TVD_FRAME_DONE=1,
    TVD_LOCK,
    TVD_UNLOCK,
}tvd_irq_t;

void f1c100s_tvd_AutoMode(void);
void f1c100s_tvd_Init(int Channel,int Mode);
void	BSP_TVD_init(void);


void BSP_TVD_irq_enable(__u32 id,tvd_irq_t irq);
__u32 BSP_TVD_irq_status_get(__u32 id,tvd_irq_t irq);
void BSP_TVD_irq_status_clear(__u32 id,tvd_irq_t irq);

void BSP_TVD_capture_on2(void);
void BSP_TVD_capture_on(__u32 id);
void BSP_TVD_capture_off(__u32 id);

void BSP_TVD_set_addr_y(__u32 id,__u32 addr);
void BSP_TVD_set_addr_c(__u32 id,__u32 addr);



void BSP_TVD_set_width(__u32 id,__u32 w);
void BSP_TVD_set_width_jump(__u32 id,__u32 j);
void BSP_TVD_set_height(__u32 id,__u32 h);
void BSP_TVD_set_ver_start(__u32 id,__u32 v_start);


//void	BSP_TVD_set_fmt(__u32 id,tvd_fmt_t fmt);
void	BSP_TVD_set_fmt(__u32 id,TVD_FMT_T fmt);
void	BSP_TVD_config(__u32 interface, __u32 system,  __u32 format);
__u32	BSP_TVD_get_status(__u32 id);
void 	BSP_TVD_3D_COMB_Filter(__u32 enable,__u32 addr);
void 	BSP_TVD_input_select(__u32 input);


void TvdDefeConfig(int Mode);
void tvd_clear_v(void);

#endif  /* _TVD_H_ */

