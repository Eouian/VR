#include "sys_interrupt.h"
#include "sys_defe.h"
#include "detab.h"
#include "sys_clock.h"
#include "sys_uart.h"
#include "sys_delay.h"
#include "sys_io.h"
//--------------
#include "sys_lcd_conf.h"
//--------------
#include "sys_tvd.h"
#include "sys_timer.h"
#include "mconf.h"

#define DEFE_Base_Address 0x01E00000

static volatile __de_scal_dev_t *scal_dev[1];
static __u32 de_scal_ch0_offset;
static __u32 de_scal_ch1_offset;
static __u32 de_scal_ch2_offset;
static __u32 de_scal_trd_fp_en = 0;
static __u32 de_scal_trd_itl_en = 0;
static __u32 de_scal_ch0r_offset;
static __u32 de_scal_ch1r_offset;
static __u32 de_scal_ch2r_offset;

/*********************************************************************************************
设置基地址
*********************************************************************************************/
__s32 DE_SCAL_Set_Reg_Base(__u8 sel, __u32 base)
{
	scal_dev[sel]=(__de_scal_dev_t *)base;
	return 0;
}

//*********************************************************************************************
// description     : scaler source concerning parameter configure
//                 addr  <frame buffer address for 3 channel, 32 bit absolute address>
//                 size <scale region define,  src size, offset, scal size>
//                 type <src data type, include byte sequence, mode, format, pixel sequence>
//                 field <frame/field data get>
//                 dien <deinterlace enable>
//***********************************************************************************************
__s32 DE_SCAL_Config_Src(__u8 sel, __scal_buf_addr_t *addr, __scal_src_size_t *size,
                         __scal_src_type_t *type, __u8 field, __u8 dien)
 {
    __u8 w_shift, h_shift;
	__u32 image_w0, image_w1;
	__u32 x_off0, y_off0, x_off1, y_off1;
	__u32 in_w0, in_h0, in_w1, in_h1;

	image_w0 = size->src_width;
	in_w0 = size->scal_width;
	in_h0 = size->scal_height;
	x_off0 = size->x_off;
	y_off0 = (field | dien) ? (size->y_off & 0xfffffffe) : size->y_off;  //scan mod enable or deinterlace, odd dy un-support

//    if(sel == 0)   //scaler 0 scaler 1
    {
        if(type->fmt == DE_SCAL_INYUV422 || type->fmt == DE_SCAL_INYUV420)
        {
            w_shift = 1;
        	image_w1 = (image_w0 + 0x1)>>w_shift;
        	in_w1 = (in_w0 + 0x1)>>w_shift;
        	x_off1 = (x_off0)>>w_shift;
        	if(type->mod == DE_SCAL_INTER_LEAVED)
        	{
        	    image_w0 = (image_w0 + 0x1) & 0xfffffffe;
                image_w1 = image_w0>>1;
                in_w0 &= 0xfffffffe;
                in_w1 = in_w0>>0x1;
        	    x_off0 &= 0xfffffffe;
        		x_off1 = x_off0>>w_shift;
        	}
        }
        else if(type->fmt == DE_SCAL_INYUV411)
        {
            w_shift = 2;
        	image_w1 = (image_w0 + 0x3)>>w_shift;
        	in_w1 = (in_w0 + 0x3)>>w_shift;
        	x_off1 = (x_off0)>>w_shift;
        }
        else
        {
            w_shift = 0;
        	image_w1 = image_w0;
        	in_w1 = in_w0;
        	x_off1 = x_off0;
        }
        if(type->fmt == DE_SCAL_INYUV420 || type->fmt == DE_SCAL_INCSIRGB)
        {
            h_shift = 1;
        	in_h1 = (in_h0 + 0x1)>>h_shift;
        	y_off1 = (y_off0)>>h_shift;
        }
        else
        {
            h_shift = 0;
        	in_h1 = in_h0;
        	y_off1 = y_off0;
        }
    }
    //added no-zero limited
    in_h0 = (in_h0!=0) ? in_h0 : 1;
	in_h1 = (in_h1!=0) ? in_h1 : 1;
	in_w0 = (in_w0!=0) ? in_w0 : 1;
	in_w1 = (in_w1!=0) ? in_w1 : 1;
	
	if(type->mod == DE_SCAL_PLANNAR)
	{
	  scal_dev[sel]->linestrd0.dwval = image_w0;
		scal_dev[sel]->linestrd1.dwval = image_w1;
		scal_dev[sel]->linestrd2.dwval = image_w1;

		if(type->fmt == DE_SCAL_INYUV420)
		{

				scal_dev[sel]->linestrd0.dwval = ((image_w0+15)>>4)<<4;
				scal_dev[sel]->linestrd1.dwval = ((image_w1+15)>>4)<<4;
				scal_dev[sel]->linestrd2.dwval = ((image_w1+15)>>4)<<4;

				scal_dev[sel]->linestrd0.dwval = ((image_w0+15)>>4)<<4;
				scal_dev[sel]->linestrd1.dwval = ((image_w1+7)>>3)<<3;
				scal_dev[sel]->linestrd2.dwval = ((image_w1+7)>>3)<<3;

		}
		de_scal_ch0_offset = image_w0 * y_off0 + x_off0;
		de_scal_ch1_offset = image_w1 * y_off1 + x_off1;
		de_scal_ch2_offset = image_w1 * y_off1 + x_off1;
		scal_dev[sel]->buf_addr0.dwval = addr->ch0_addr+ de_scal_ch0_offset;
		scal_dev[sel]->buf_addr1.dwval = addr->ch1_addr+ de_scal_ch1_offset;
		scal_dev[sel]->buf_addr2.dwval = addr->ch2_addr+ de_scal_ch2_offset;
	}
	else if(type->mod == DE_SCAL_INTER_LEAVED)
	{
	    scal_dev[sel]->linestrd0.dwval = image_w0<<(0x2 - w_shift);
		scal_dev[sel]->linestrd1.dwval = 0x0;
		scal_dev[sel]->linestrd2.dwval = 0x0;

        de_scal_ch0_offset = ((image_w0 * y_off0 + x_off0)<<(0x2 - w_shift));
        de_scal_ch1_offset = 0x0;
        de_scal_ch2_offset = 0x0;
		scal_dev[sel]->buf_addr0.dwval = addr->ch0_addr+ de_scal_ch0_offset;
		scal_dev[sel]->buf_addr1.dwval = addr->ch1_addr;
		scal_dev[sel]->buf_addr2.dwval = addr->ch2_addr;
	}
	else if(type->mod == DE_SCAL_UVCOMBINED)
	{
	    scal_dev[sel]->linestrd0.dwval = image_w0;
		scal_dev[sel]->linestrd1.dwval = image_w1<<1;
		scal_dev[sel]->linestrd2.dwval = 0x0;

        de_scal_ch0_offset = image_w0 * y_off0 + x_off0;
        de_scal_ch1_offset = (((image_w1) * (y_off1) + (x_off1))<<1);
        de_scal_ch2_offset = 0x0;
		scal_dev[sel]->buf_addr0.dwval = addr->ch0_addr+ de_scal_ch0_offset;
		scal_dev[sel]->buf_addr1.dwval = addr->ch1_addr+ de_scal_ch1_offset;
		scal_dev[sel]->buf_addr2.dwval = addr->ch2_addr;
	}
	else if(type->mod == DE_SCAL_PLANNARMB)
	{
	    image_w0 = (image_w0 + 0xf)&0xfff0;
        image_w1 = (image_w1 + (0xf>>w_shift)) & (~(0xf>>w_shift));
		
        //block offset
        scal_dev[sel]->mb_off0.bits.x_offset0 = (x_off0 & 0x0f);
        scal_dev[sel]->mb_off0.bits.y_offset0 = (y_off0 & 0x0f);
        scal_dev[sel]->mb_off0.bits.x_offset1 = (((x_off0 & 0x0f) & (0x0f)) + in_w0 + 0x0f) & 0x0f;
        scal_dev[sel]->mb_off1.bits.x_offset0 = ((x_off1)&(0x0f>>w_shift));
        scal_dev[sel]->mb_off1.bits.y_offset0 = ((y_off1)&(0x0f>>h_shift));
        scal_dev[sel]->mb_off1.bits.x_offset1 = ((((x_off1)&(0x0f>>w_shift)) & (0x0f>>w_shift)) + (in_w1) + (0x0f>>w_shift))&(0x0f>>w_shift);
		scal_dev[sel]->mb_off2.bits.x_offset0 = scal_dev[sel]->mb_off1.bits.x_offset0;
		scal_dev[sel]->mb_off2.bits.y_offset0 = scal_dev[sel]->mb_off1.bits.y_offset0;
		scal_dev[sel]->mb_off2.bits.x_offset1 = scal_dev[sel]->mb_off1.bits.x_offset1;

		scal_dev[sel]->linestrd0.dwval = (image_w0 - 0xf)<<4;
		scal_dev[sel]->linestrd1.dwval = ((image_w1) <<(0x04-h_shift)) - ((0xf>>h_shift)<<(0x04-w_shift));
		scal_dev[sel]->linestrd2.dwval = scal_dev[sel]->linestrd1.dwval;

        de_scal_ch0_offset = ((image_w0 + 0x0f)&0xfff0) * (y_off0&0xfff0) + ((y_off0&0x00f)<<4) + ((x_off0&0xff0)<<4);
        de_scal_ch1_offset = (((image_w1) + (0x0f>>w_shift)) &(0xfff0>>w_shift)) * ((y_off1) & (0xfff0>>h_shift)) + 
                             ((((y_off1) & (0x00f>>h_shift))<<(0x4 - w_shift))) + (((x_off1) & (0xfff0>>w_shift))<<(0x4 - h_shift));
        de_scal_ch2_offset = de_scal_ch1_offset;
		scal_dev[sel]->buf_addr0.dwval = addr->ch0_addr+ de_scal_ch0_offset;
		scal_dev[sel]->buf_addr1.dwval = addr->ch1_addr+ de_scal_ch1_offset;
		scal_dev[sel]->buf_addr2.dwval = addr->ch2_addr+ de_scal_ch2_offset;
	}
	else if(type->mod == DE_SCAL_UVCOMBINEDMB)
	{
	    image_w0 = (image_w0 + 0x1f)&0xffffffe0;
		image_w1 = (image_w1 + 0x0f)&0xfffffff0;
		//block offset
		scal_dev[sel]->mb_off0.bits.x_offset0 = (x_off0 & 0x1f);
    scal_dev[sel]->mb_off0.bits.y_offset0 = (y_off0 & 0x1f);
		scal_dev[sel]->mb_off0.bits.x_offset1 = (((x_off0 & 0x1f) & 0x1f) + in_w0 + 0x1f) &0x1f;
		scal_dev[sel]->mb_off1.bits.x_offset0 = (((x_off1)<<1)&0x1f);
    scal_dev[sel]->mb_off1.bits.y_offset0 = ((y_off1)&0x1f);
    scal_dev[sel]->mb_off1.bits.x_offset1 = (((((x_off1)<<1)&0x1f) & 0x1e) + ((in_w1)<<1) + 0x1f) & 0x1f; 

		scal_dev[sel]->linestrd0.dwval = (((image_w0 + 0x1f)&0xffe0) - 0x1f)<<0x05;
    scal_dev[sel]->linestrd1.dwval = (((((image_w1)<<1)+0x1f)&0xffe0) - 0x1f) << 0x05;
    scal_dev[sel]->linestrd2.dwval = 0x00;

    de_scal_ch0_offset = ((image_w0 + 0x1f) &0xffe0) * (y_off0& 0xffe0) + ((y_off0& 0x01f)<<5) + ((x_off0& 0xffe0)<<5);
    de_scal_ch1_offset = (((image_w1<< 0x01)+0x1f)&0xffe0) * ((y_off1) & 0xffe0) + (((y_off1) & 0x01f)<<5) + (((x_off1<<0x01) & 0xffe0)<<5);
    de_scal_ch2_offset = 0x0;
		scal_dev[sel]->buf_addr0.dwval = addr->ch0_addr+ de_scal_ch0_offset;
    scal_dev[sel]->buf_addr1.dwval = addr->ch1_addr+ de_scal_ch1_offset;
    scal_dev[sel]->buf_addr2.dwval = 0x0; 
	}

	scal_dev[sel]->input_fmt.bits.byte_seq = type->byte_seq;
	scal_dev[sel]->input_fmt.bits.data_mod = type->mod;
	scal_dev[sel]->input_fmt.bits.data_fmt = type->fmt;
	scal_dev[sel]->input_fmt.bits.data_ps = type->ps;

	scal_dev[sel]->ch0_insize.bits.in_width = in_w0 - 1;
	scal_dev[sel]->ch0_insize.bits.in_height = in_h0 - 1;
	scal_dev[sel]->ch1_insize.bits.in_width = in_w1 - 1;
	scal_dev[sel]->ch1_insize.bits.in_height = in_h1 - 1;

	scal_dev[sel]->trd_ctrl.dwval = 0;
    return 0;
}

//*********************************************************************************************
// description     : set scaler init phase according to in/out information
//                 in_scan <scale src data scan mode, if deinterlaceing open, the scan mode is progressive for scale>
//                 in_size <scale region define,  src size, offset, scal size>
//                 in_type <src data type>
//                 out_scan <scale output data scan mode>
//                 out_size <scale out size>
//                 out_type <output data format>
//                 dien <deinterlace enable>
//note               : when 3D mode(when output mode is HDMI_FPI), the Function Set_3D_Ctrl msut carry out first. 
//                         when 3D mode(HDMI_FPI), this function used once
//***********************************************************************************************
__s32 DE_SCAL_Set_Init_Phase(__u8 sel, __scal_scan_mod_t *in_scan, __scal_src_size_t *in_size,
                             __scal_src_type_t *in_type, __scal_scan_mod_t *out_scan,
                             __scal_out_size_t *out_size, __scal_out_type_t *out_type, __u8 dien)
 {
     __s32 ch0_h_phase=0, ch0_v_phase0=0, ch0_v_phase1=0, ch12_h_phase=0, ch12_v_phase0=0, ch12_v_phase1=0;
	 __u8 h_shift=0, w_shift=0;
     __s32 in_h0, in_h1, out_h0, out_h1;

     //set register value
	 scal_dev[sel]->output_fmt.bits.scan_mod = out_scan->field;
     scal_dev[sel]->input_fmt.bits.scan_mod = out_scan->field ? 0x0 : in_scan->field;  //out scan and in scan are not valid at the same time
     if(de_scal_trd_itl_en == 0)   //added for 3D top_bottom mode, zchmin 2011-05-04, note: when HDMI_FPI, the input inscan mode must open, 
 	{
		 scal_dev[sel]->field_ctrl.bits.field_loop_mod = 0x0;
		 scal_dev[sel]->field_ctrl.bits.valid_field_cnt = 0x1-0x1;
		 scal_dev[sel]->field_ctrl.bits.field_cnt = in_scan->bottom;
 	}  
     //sampling method, phase
	 if(in_type->fmt == DE_SCAL_INYUV420)
	 {
	     if(in_type->sample_method == 0x0)  //
	     {
	         ch0_h_phase = 0x0;
			 ch0_v_phase0 = 0x0;
			 ch0_v_phase1 = 0x0;
			 ch12_h_phase = 0xfc000;   //-0.25
			 ch12_v_phase0 = 0xfc000;  //-0.25
			 ch12_v_phase1 = 0xfc000;  //-0.25
	     }
		 else
		 {
		     ch0_h_phase = 0x0;
			 ch0_v_phase0 = 0x0;
			 ch0_v_phase1 = 0x0;
			 ch12_h_phase = 0x0;       //0
			 ch12_v_phase0 = 0xfc000;  //-0.25
			 ch12_v_phase1 = 0xfc000;  //-0.25
		 }
	 }
	 else  //can added yuv411 or yuv420 init phase for sample method
	 {
	     ch0_h_phase = 0x0;
		 ch0_v_phase0 = 0x0;
		 ch0_v_phase1 = 0x0;
		 ch12_h_phase = 0x0;
		 ch12_v_phase0 = 0x0;
		 ch12_v_phase1 = 0x0; 
	 }

     //location offset
     w_shift = (in_type->fmt == DE_SCAL_INYUV420 || in_type->fmt == DE_SCAL_INYUV422) ? 0x1 : ((in_type->fmt == DE_SCAL_INYUV411) ? 0x2 : 0x0);
	 h_shift = (in_type->fmt == DE_SCAL_INYUV420 || in_type->fmt == DE_SCAL_INCSIRGB) ? 0x1 : 0x0;

     //deinterlace and in scan mode enable, //dy
     if(((dien == 0x01) || (in_scan->field== 0x1)) && (in_size->y_off & 0x1)&& (in_scan->bottom == 0x0))  //
     {
         ch0_v_phase0 = (ch0_v_phase0 + 0x10000) & SCALINITPASELMT;
         ch12_v_phase0 = (ch12_v_phase0 + 0x10000) & SCALINITPASELMT;
     }
     else
     {
         ch12_v_phase0 = (ch12_v_phase0 + (in_size->y_off & ((1<<h_shift)-1))*(0x10000>>h_shift)) & SCALINITPASELMT;
         ch12_v_phase1 = ch12_v_phase0;
     }
	 
	 //dx
	 scal_dev[sel]->ch0_horzphase.bits.phase = ch0_h_phase;
	 scal_dev[sel]->ch1_horzphase.bits.phase = (ch12_h_phase + (in_size->x_off & ((1<<w_shift) - 1)) * (0x10000>>w_shift)) & SCALINITPASELMT;

     //outinterlace,
     if(out_scan->field == 0x1)  //outinterlace enable
     {
         in_h0 = in_size->scal_height;
         in_h1 = (in_type->fmt == DE_SCAL_INYUV420) ? (in_h0+0x1)>>1: in_h0;
         out_h0 = out_size->height;
         out_h1 = (out_type->fmt == DE_SCAL_OUTPYUV420) ? (out_h0+0x1)>>1 : out_h0;

		 //added no-zero limited
		in_h0 = (in_h0!=0) ? in_h0 : 1;
		in_h1 = (in_h1!=0) ? in_h1 : 1;
		out_h0 = (out_h0!=0) ? out_h0 : 1;
		out_h1 = (out_h1!=0) ? out_h1 : 1;
			 
         if(in_scan->bottom == 0x0)
         {
	         scal_dev[sel]->ch0_vertphase0.bits.phase = ch0_v_phase0;
             scal_dev[sel]->ch0_vertphase1.bits.phase = ch0_v_phase0 + ((in_h0>>in_scan->field)<<16)/(out_h0);
             scal_dev[sel]->ch1_vertphase0.bits.phase = ch12_v_phase0;
             scal_dev[sel]->ch1_vertphase1.bits.phase = ch12_v_phase0 + ((in_h1>>in_scan->field)<<16)/(out_h1);
         }
         else
         {
             scal_dev[sel]->ch0_vertphase0.bits.phase = ch0_v_phase1;
             scal_dev[sel]->ch0_vertphase1.bits.phase = ch0_v_phase1 + ((in_h0>>in_scan->field)<<16)/(out_h0);
             scal_dev[sel]->ch1_vertphase0.bits.phase = ch12_v_phase1;
             scal_dev[sel]->ch1_vertphase1.bits.phase = ch12_v_phase1 + ((in_h1>>in_scan->field)<<16)/(out_h1);
         }
     }
     else  //outinterlace disable
     {
         scal_dev[sel]->ch0_vertphase0.bits.phase = ch0_v_phase0;
         scal_dev[sel]->ch0_vertphase1.bits.phase = ch0_v_phase1;
         scal_dev[sel]->ch1_vertphase0.bits.phase = ch12_v_phase0;
         scal_dev[sel]->ch1_vertphase1.bits.phase = ch12_v_phase1;
         
     }     	 
	 return 0;
}
//*********************************************************************************************
// description      : set scaler scaling factor, modify algorithm and tape offset
//                 in_scan <scale src data scan mode, if deinterlaceing open, the scan mode is progressive for scale>
//                 in_size <scale region define,  src size, offset, scal size>
//                 in_type <src data type>
//                 out_scan <scale output data scan mode>
//                 out_size <scale out size, when output interlace, the height is 2xoutheight ,for example 480i, the value is 480>
//                 out_type <output data format>
//history           :  2011/03/31  modify channel 1/2 scaling factor
//***********************************************************************************************                                   
__s32 DE_SCAL_Set_Scaling_Factor(__u8 sel, __scal_scan_mod_t *in_scan, __scal_src_size_t *in_size,
                                 __scal_src_type_t *in_type, __scal_scan_mod_t *out_scan, 
                                 __scal_out_size_t *out_size, __scal_out_type_t *out_type)
{
    __s32 in_w0, in_h0, out_w0, out_h0;
    __s32 ch0_hstep, ch0_vstep ;
	__s8 w_shift, h_shift;
    
    in_w0 = in_size->scal_width;
    in_h0 = in_size->scal_height;

    out_w0 = out_size->width;
    out_h0 = out_size->height + (out_scan->field & 0x1);	//modify by zchmin 110317

	//sc0 
	if((in_type->mod == DE_SCAL_INTER_LEAVED) && (in_type->fmt == DE_SCAL_INYUV422))
    {
        in_w0 &=0xfffffffe;
    }
    //algorithm select
    if(out_w0 > SCALLINEMAX)
    {
	    scal_dev[sel]->agth_sel.bits.linebuf_agth = 0x1;
        if(in_w0>SCALLINEMAX)  //
        {
            in_w0 = SCALLINEMAX;
        }
    }
    else
    {
        scal_dev[sel]->agth_sel.bits.linebuf_agth= 0x0;
    }

    w_shift = (in_type->fmt == DE_SCAL_INYUV411) ? 2 : ((in_type->fmt == DE_SCAL_INYUV420)||(in_type->fmt == DE_SCAL_INYUV422)) ? 1 : 0;
	h_shift = ((in_type->fmt == DE_SCAL_INYUV420) || (in_type->fmt == DE_SCAL_INCSIRGB)) ? 1 : 0;
		
    
    if((out_type->fmt == DE_SCAL_OUTPYUV420) || (out_type->fmt == DE_SCAL_OUTPYUV422))
    {
        w_shift -= 1 ;
    }
    else if(out_type->fmt == DE_SCAL_OUTPYUV411)
    {
        w_shift -= 2 ;
    }
    else
    {
        w_shift -= 0 ;;
    }
    if(out_type->fmt == DE_SCAL_OUTPYUV420)
    {
        h_shift -= 1;
    }
    else
    {
        h_shift -= 0;
    }
	//added no-zero limited
    in_h0 = (in_h0!=0) ? in_h0 : 1;
	in_w0 = (in_w0!=0) ? in_w0 : 1;
	out_h0 = (out_h0!=0) ? out_h0 : 1;
	out_w0 = (out_w0!=0) ? out_w0 : 1;
	
    //step factor
		
//		sysprintf("s1=%d\r\n",in_w0);
//		sysprintf("s2=%d\r\n",out_w0);
//		sysprintf("s3=%d\r\n",in_w0<<16);
//		sysprintf("s3=%x\r\n",(in_w0<<16)/out_w0);
//		
//		
    ch0_hstep = (in_w0<<16)/out_w0;
    ch0_vstep = ((in_h0>>in_scan->field)<<16)/( out_h0 );
    
	  scal_dev[sel]->ch0_horzfact.dwval = ch0_hstep;
    scal_dev[sel]->ch0_vertfact.dwval = ch0_vstep<<(out_scan->field);
    scal_dev[sel]->ch1_horzfact.dwval = (w_shift>0) ? (ch0_hstep>>w_shift) : ch0_hstep<<(0-w_shift);
    scal_dev[sel]->ch1_vertfact.dwval = (h_shift>0) ? (ch0_vstep>>h_shift)<<(out_scan->field) : (ch0_vstep<<(0-h_shift))<<(out_scan->field);
    
	return 0;
}
//*********************************************************************************************
// description      : set scaler scaling filter coefficients
//                 in_scan <scale src data scan mode, if deinterlaceing open, the scan mode is progressive for scale>
//                 in_size <scale region define,  src size, offset, scal size>
//                 in_type <src data type>
//                 out_scan <scale output data scan mode>
//                 out_size <scale out size>
//                 out_type <output data format>
//                 smth_mode <scaler filter effect select>
//***********************************************************************************************                                        
__s32 DE_SCAL_Set_Scaling_Coef(__u8 sel, __scal_scan_mod_t *in_scan, __scal_src_size_t *in_size,
                               __scal_src_type_t *in_type, __scal_scan_mod_t *out_scan, 
                               __scal_out_size_t *out_size, __scal_out_type_t *out_type, __u8 smth_mode)  
{
    __s32 in_w0, in_h0, in_w1, in_h1, out_w0, out_h0, out_w1, out_h1;
    __s32 ch0h_smth_level, ch0v_smth_level, ch1h_smth_level, ch1v_smth_level;
    __u32 int_part, float_part;
    __u32 zoom0_size, zoom1_size, zoom2_size, zoom3_size, zoom4_size, zoom5_size, al1_size;
    __u32 ch0h_sc, ch0v_sc, ch1h_sc, ch1v_sc;
    __u32 ch0v_fir_coef_addr, ch0h_fir_coef_addr, ch1v_fir_coef_addr, ch1h_fir_coef_addr;
    __u32 ch0v_fir_coef_ofst, ch0h_fir_coef_ofst, ch1v_fir_coef_ofst, ch1h_fir_coef_ofst;
    __s32 fir_ofst_tmp;
    __u32 i;

    __u32 loop_count = 0;
    in_w0 = in_size->scal_width;
    in_h0 = in_size->scal_height;

    out_w0 = out_size->width;
    out_h0 = out_size->height;

    zoom0_size = 1;
    zoom1_size = 8;
    zoom2_size = 4;
    zoom3_size = 1;
    zoom4_size = 1;
    zoom5_size = 1;
    al1_size = zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size + zoom5_size;
    
    if((in_type->mod == DE_SCAL_INTER_LEAVED) && (in_type->fmt == DE_SCAL_INYUV422))
    {
        in_w0 &=0xfffffffe;
    }
    
    //channel 1,2 size 
    if((in_type->fmt == DE_SCAL_INYUV420) || (in_type->fmt == DE_SCAL_INYUV422))
    {
        in_w1 = (in_w0 + 0x1)>>0x1;
    }
    else if(in_type->fmt == DE_SCAL_INYUV411)
    {
        in_w1 = (in_w0 + 0x3)>>0x2;
    }
    else
    {
        in_w1 = in_w0;
    }
    if((in_type->fmt == DE_SCAL_INYUV420) || (in_type->fmt == DE_SCAL_INCSIRGB))
    {
        in_h1 = (in_h0 + 0x1)>>0x1;
    }
    else
    {
        in_h1 = in_h0;
    }
    if((out_type->fmt == DE_SCAL_OUTPYUV420) || (out_type->fmt == DE_SCAL_OUTPYUV422))
    {
        out_w1 = (out_w0 + 0x1)>>0x1;
    }
    else if(out_type->fmt == DE_SCAL_OUTPYUV411)
    {
        out_w1 = (out_w0 + 0x3)>>0x2;
    }
    else
    {
        out_w1 = out_w0;
    }
    if(out_type->fmt == DE_SCAL_OUTPYUV420)
    {
        out_h1 = (out_h0+ 0x1)>>0x1;
    }
    else
    {
        out_h1 = out_h0;
    }
    
    //added no-zero limited
    in_h0 = (in_h0!=0) ? in_h0 : 1;
	in_h1 = (in_h1!=0) ? in_h1 : 1;
	in_w0 = (in_w0!=0) ? in_w0 : 1;
	in_w1 = (in_w1!=0) ? in_w1 : 1;
	out_h0 = (out_h0!=0) ? out_h0 : 1;
	out_h1 = (out_h1!=0) ? out_h1 : 1;
	out_w0 = (out_w0!=0) ? out_w0 : 1;
	out_w1 = (out_w1!=0) ? out_w1 : 1;
	    
    //smooth level for channel 0,1 in vertical and horizontal direction
    ch0h_smth_level = (smth_mode&0x40)  ?  0 - (smth_mode&0x3f) : smth_mode&0x3f;
    ch0v_smth_level = ch0h_smth_level;
    if((smth_mode>>7) &0x01)  
    {
      ch0v_smth_level = (smth_mode&0x4000) ? 0 - ((smth_mode&0x3f00)>>8) : ((smth_mode&0x3f00)>>8);
    }
    if((smth_mode>>31)&0x01)
    {
      ch1h_smth_level = (smth_mode&0x400000) ? 0 - ((smth_mode&0x3f0000)>>16) : ((smth_mode&0x3f0000)>>16);
      ch1v_smth_level = ch1h_smth_level;
      if((smth_mode >> 23)&0x1)
      {
        ch1v_smth_level = (smth_mode&0x40000000) ? 0 - ((smth_mode&0x3f000000)>>24) : ((smth_mode&0x3f000000)>>24);
      }
    }
    //
    ch0h_sc = (in_w0<<3)/out_w0;
    ch0v_sc = (in_h0<<(3-in_scan->field))/(out_h0);
    ch1h_sc = (in_w1<<3)/out_w1;
    ch1v_sc = (in_h1<<(3-in_scan->field))/(out_h1);

    //modify ch1 smooth level according to ratio to ch0
    if(((smth_mode>>31)&0x01)==0x0)
    {
      if(!ch1h_sc)
      {
        ch1h_smth_level = 0;
      }
      else
      {
        ch1h_smth_level = ch0h_smth_level>>(ch0h_sc/ch1h_sc);
      }

      if(!ch1v_sc)
      {
        ch1v_smth_level = 0;
      }
      else
      {
        ch1v_smth_level = ch0v_smth_level>>(ch0v_sc/ch1v_sc);
      }
    }
      
      //comput the fir coefficient offset in coefficient table
      int_part = ch0v_sc>>3;
      float_part = ch0v_sc & 0x7;
      ch0v_fir_coef_ofst = (int_part==0)  ? zoom0_size : 
                           (int_part==1)  ? zoom0_size + float_part :
                           (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                           (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                           (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                           zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
      int_part = ch0h_sc>>3;
      float_part = ch0h_sc & 0x7;
      ch0h_fir_coef_ofst = (int_part==0)  ? zoom0_size : 
                           (int_part==1)  ? zoom0_size + float_part :
                           (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                           (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                           (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                           zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
      int_part = ch1v_sc>>3;
      float_part = ch1v_sc & 0x7;
      ch1v_fir_coef_ofst = (int_part==0)  ? zoom0_size : 
                           (int_part==1)  ? zoom0_size + float_part :
                           (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                           (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                           (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                           zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
      int_part = ch1h_sc>>3;
      float_part = ch1h_sc & 0x7;
      ch1h_fir_coef_ofst =  (int_part==0)  ? zoom0_size : 
                            (int_part==1)  ? zoom0_size + float_part :
                            (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                            (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                            (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                            zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
    //added smooth level for each channel in horizontal and vertical direction
    fir_ofst_tmp = ch0v_fir_coef_ofst + ch0v_smth_level;
    ch0v_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    fir_ofst_tmp = ch0h_fir_coef_ofst + ch0h_smth_level;
    ch0h_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    fir_ofst_tmp = ch1v_fir_coef_ofst + ch1v_smth_level;
    ch1v_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    fir_ofst_tmp = ch1h_fir_coef_ofst + ch1h_smth_level;
    ch1h_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    //modify coefficient offset
    ch0v_fir_coef_ofst = (ch0v_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch0v_fir_coef_ofst;
    ch1v_fir_coef_ofst = (ch1v_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch1v_fir_coef_ofst;
    ch0h_fir_coef_ofst = (ch0h_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch0h_fir_coef_ofst; 
    ch1h_fir_coef_ofst = (ch1h_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch1h_fir_coef_ofst;                                           
    
    /* for  single buffer */
    ch0v_fir_coef_addr = (ch0v_fir_coef_ofst<<7);
    ch0h_fir_coef_addr = (ch0h_fir_coef_ofst<<7);
    ch1v_fir_coef_addr = (ch1v_fir_coef_ofst<<7);
    ch1h_fir_coef_addr = (ch1h_fir_coef_ofst<<7);
    scal_dev[sel]->frm_ctrl.bits.coef_access_ctrl= 1;
    while((scal_dev[sel]->status.bits.coef_access_status == 0) && (loop_count < 20)) {
        loop_count ++;
    }

    for(i=0; i<32; i++)
    {
      scal_dev[sel]->ch0_horzcoef0[i].dwval = fir_tab[(ch0h_fir_coef_addr>>2) + i];
      scal_dev[sel]->ch0_vertcoef[i].dwval  = fir_tab[(ch0v_fir_coef_addr>>2) + i];
      scal_dev[sel]->ch1_horzcoef0[i].dwval = fir_tab[(ch1h_fir_coef_addr>>2) + i];
      scal_dev[sel]->ch1_vertcoef[i].dwval  = fir_tab[(ch1v_fir_coef_addr>>2) + i];
    }

    scal_dev[sel]->frm_ctrl.bits.coef_access_ctrl = 0x0;

    return 0;
}
//*********************************************************************************************
// description      : set scaler scaling filter coefficients for video playback
// parameters       :
//                 sel <scaler select>
//                 in_scan <scale src data scan mode, if deinterlaceing open, the scan mode is progressive for scale>
//                 in_size <scale region define,  src size, offset, scal size>
//                 in_type <src data type>
//                 out_scan <scale output data scan mode>
//                 out_size <scale out size>
//                 out_type <output data format>
//                 smth_mode <scaler filter effect select>
//***********************************************************************************************                                        
__s32 DE_SCAL_Set_Scaling_Coef_for_video(__u8 sel, __scal_scan_mod_t *in_scan, __scal_src_size_t *in_size,
                               __scal_src_type_t *in_type, __scal_scan_mod_t *out_scan, 
                               __scal_out_size_t *out_size, __scal_out_type_t *out_type, __u32 smth_mode)  
{
    __s32 in_w0, in_h0, in_w1, in_h1, out_w0, out_h0, out_w1, out_h1;
    __s32 ch0h_smth_level=0, ch0v_smth_level=0, ch1h_smth_level=0, ch1v_smth_level=0;
    __u32 int_part, float_part;
    __u32 zoom0_size, zoom1_size, zoom2_size, zoom3_size, zoom4_size, zoom5_size, al1_size;
    __u32 ch0h_sc, ch0v_sc, ch1h_sc, ch1v_sc;
    __u32 ch0v_fir_coef_addr, ch0h_fir_coef_addr, ch1v_fir_coef_addr, ch1h_fir_coef_addr;
    __u32 ch0v_fir_coef_ofst, ch0h_fir_coef_ofst, ch1v_fir_coef_ofst, ch1h_fir_coef_ofst;
    __s32 fir_ofst_tmp;
    __u32 i;
    
    in_w0 = in_size->scal_width;
    in_h0 = in_size->scal_height;

    out_w0 = out_size->width;
    out_h0 = out_size->height;

    zoom0_size = 1;
    zoom1_size = 8;
    zoom2_size = 4;
    zoom3_size = 1;
    zoom4_size = 1;
    zoom5_size = 1;
    al1_size = zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size + zoom5_size;
    
    if((in_type->mod == DE_SCAL_INTER_LEAVED) && (in_type->fmt == DE_SCAL_INYUV422))
    {
        in_w0 &=0xfffffffe;
    }
    
    //channel 1,2 size 
    if((in_type->fmt == DE_SCAL_INYUV420) || (in_type->fmt == DE_SCAL_INYUV422))
    {
        in_w1 = (in_w0 + 0x1)>>0x1;
    }
    else if(in_type->fmt == DE_SCAL_INYUV411)
    {
        in_w1 = (in_w0 + 0x3)>>0x2;
    }
    else
    {
        in_w1 = in_w0;
    }
    if((in_type->fmt == DE_SCAL_INYUV420) || (in_type->fmt == DE_SCAL_INCSIRGB))
    {
        in_h1 = (in_h0 + 0x1)>>0x1;
    }
    else
    {
        in_h1 = in_h0;
    }
    if((out_type->fmt == DE_SCAL_OUTPYUV420) || (out_type->fmt == DE_SCAL_OUTPYUV422))
    {
        out_w1 = (out_w0 + 0x1)>>0x1;
    }
    else if(out_type->fmt == DE_SCAL_OUTPYUV411)
    {
        out_w1 = (out_w0 + 0x3)>>0x2;
    }
    else
    {
        out_w1 = out_w0;
    }
    if(out_type->fmt == DE_SCAL_OUTPYUV420)
    {
        out_h1 = (out_h0+ 0x1)>>0x1;
    }
    else
    {
        out_h1 = out_h0;
    }
    
    //added no-zero limited
    in_h0 = (in_h0!=0) ? in_h0 : 1;
	in_h1 = (in_h1!=0) ? in_h1 : 1;
	in_w0 = (in_w0!=0) ? in_w0 : 1;
	in_w1 = (in_w1!=0) ? in_w1 : 1;
	out_h0 = (out_h0!=0) ? out_h0 : 1;
	out_h1 = (out_h1!=0) ? out_h1 : 1;
	out_w0 = (out_w0!=0) ? out_w0 : 1;
	out_w1 = (out_w1!=0) ? out_w1 : 1;
	    
    //smooth level for channel 0,1 in vertical and horizontal direction
    ch0h_smth_level = (smth_mode&0x40)  ?  0 - (smth_mode&0x3f) : smth_mode&0x3f;
    ch0v_smth_level = ch0h_smth_level;
    if((smth_mode>>7) &0x01)  
    {
      ch0v_smth_level = (smth_mode&0x4000) ? 0 - ((smth_mode&0x3f00)>>8) : ((smth_mode&0x3f00)>>8);
    }
    if((smth_mode>>31)&0x01)
    {
      ch1h_smth_level = (smth_mode&0x400000) ? 0 - ((smth_mode&0x3f0000)>>16) : ((smth_mode&0x3f0000)>>16);
      ch1v_smth_level = ch1h_smth_level;
      if((smth_mode >> 23)&0x1)
      {
        ch1v_smth_level = (smth_mode&0x40000000) ? 0 - ((smth_mode&0x3f000000)>>24) : ((smth_mode&0x3f000000)>>24);
      }
    }
    //
    ch0h_sc = (in_w0<<3)/out_w0;
    ch0v_sc = (in_h0<<(3-in_scan->field))/(out_h0);
    ch1h_sc = (in_w1<<3)/out_w1;
    ch1v_sc = (in_h1<<(3-in_scan->field))/(out_h1);

    //modify ch1 smooth level according to ratio to ch0
    if(((smth_mode>>31)&0x01)==0x0)
    {
      if(!ch1h_sc)
      {
        ch1h_smth_level = 0;
      }
      else
      {
        ch1h_smth_level = ch0h_smth_level>>(ch0h_sc/ch1h_sc);
      }

      if(!ch1v_sc)
      {
        ch1v_smth_level = 0;
      }
      else
      {
        ch1v_smth_level = ch0v_smth_level>>(ch0v_sc/ch1v_sc);
      }
    }
      
      //comput the fir coefficient offset in coefficient table
      int_part = ch0v_sc>>3;
      float_part = ch0v_sc & 0x7;
      ch0v_fir_coef_ofst = (int_part==0)  ? 0 : 					//modify by vito 12-04-16
                           (int_part==1)  ? zoom0_size + float_part :
                           (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                           (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                           (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                           zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
      int_part = ch0h_sc>>3;
      float_part = ch0h_sc & 0x7;
      ch0h_fir_coef_ofst = (int_part==0)  ? 0 : 					//modify by vito 12-04-16
                           (int_part==1)  ? zoom0_size + float_part :
                           (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                           (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                           (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                           zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
      int_part = ch1v_sc>>3;
      float_part = ch1v_sc & 0x7;
      ch1v_fir_coef_ofst = (int_part==0)  ? 0 : 					//modify by vito 12-04-16
                           (int_part==1)  ? zoom0_size + float_part :
                           (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                           (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                           (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                           zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
      int_part = ch1h_sc>>3;
      float_part = ch1h_sc & 0x7;
      ch1h_fir_coef_ofst =  (int_part==0)  ? 0 : 					//modify by vito 12-04-16
                            (int_part==1)  ? zoom0_size + float_part :
                            (int_part==2)  ? zoom0_size + zoom1_size + (float_part>>1) : 
                            (int_part==3)  ? zoom0_size + zoom1_size + zoom2_size : 
                            (int_part==4)  ? zoom0_size + zoom1_size + zoom2_size +zoom3_size : 
                            zoom0_size + zoom1_size + zoom2_size + zoom3_size + zoom4_size;
    //added smooth level for each channel in horizontal and vertical direction
    fir_ofst_tmp = ch0v_fir_coef_ofst + ch0v_smth_level;
    ch0v_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    fir_ofst_tmp = ch0h_fir_coef_ofst + ch0h_smth_level;
    ch0h_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    fir_ofst_tmp = ch1v_fir_coef_ofst + ch1v_smth_level;
    ch1v_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    fir_ofst_tmp = ch1h_fir_coef_ofst + ch1h_smth_level;
    ch1h_fir_coef_ofst = (fir_ofst_tmp<0) ? 0 : fir_ofst_tmp;
    //modify coefficient offset
    ch0v_fir_coef_ofst = (ch0v_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch0v_fir_coef_ofst;
    ch1v_fir_coef_ofst = (ch1v_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch1v_fir_coef_ofst;
    ch0h_fir_coef_ofst = (ch0h_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch0h_fir_coef_ofst; 
    ch1h_fir_coef_ofst = (ch1h_fir_coef_ofst > (al1_size - 1)) ? (al1_size - 1) : ch1h_fir_coef_ofst;                                           
    
    //compute the fir coeficient address for each channel in horizontal and vertical direction
    ch0v_fir_coef_addr = (ch0v_fir_coef_ofst<<7);
    ch0h_fir_coef_addr = ((al1_size)<<7) + (ch0h_fir_coef_ofst<<8);
    ch1v_fir_coef_addr = (ch1v_fir_coef_ofst<<7);
    ch1h_fir_coef_addr = ((al1_size)<<7) + (ch1h_fir_coef_ofst<<8);

    for(i=0; i<32; i++)
    {
	    scal_dev[sel]->ch0_horzcoef0[i].dwval = fir_tab_video[(ch0h_fir_coef_addr>>2) + 2*i];
		  scal_dev[sel]->ch0_horzcoef1[i].dwval = fir_tab_video[(ch0h_fir_coef_addr>>2) + 2*i + 1];
      scal_dev[sel]->ch0_vertcoef[i].dwval  = fir_tab_video[(ch0v_fir_coef_addr>>2) + i];
		  scal_dev[sel]->ch1_horzcoef0[i].dwval = fir_tab_video[(ch1h_fir_coef_addr>>2) + 2*i];
		  scal_dev[sel]->ch1_horzcoef1[i].dwval = fir_tab_video[(ch1h_fir_coef_addr>>2) + 2*i + 1];
      scal_dev[sel]->ch1_vertcoef[i].dwval  = fir_tab_video[(ch1v_fir_coef_addr>>2) + i];
    }

    scal_dev[sel]->frm_ctrl.bits.coef_rdy_en = 0x1;
      
    return 0;
}


//*********************************************************************************************
// description      : set scaler input/output color space convert coefficients
//                 in_csc_mode <color space select, bt601, bt709, ycc, xycc>
//                 out_csc_mode <color space select, bt601, bt709, ycc, xycc>
//                 incs <source color space>
//                 |    0  rgb
//                 |    1  yuv
//                 outcs <destination color space>
//                 |    0  rgb
//                 |    1  yuv
//                 in_br_swap <swap b r component>
//                 |    0  normal
//                 |    1  swap enable, note: when input yuv, then u v swap
//                 out_br_swap <swap output b r component>
//                 |    0  normal
//                 |    1  swap enable, note: when output yuv, then u v swap
//*********************************************************************************************** 
__s32 DE_SCAL_Set_CSC_Coef(__u8 sel, __u8 in_csc_mode, __u8 out_csc_mode, __u8 incs, __u8 outcs, __u32  in_br_swap, __u32 out_br_swap)                                                                
{
    __u8  csc_pass;
    __u32 csc_coef_addr;
    __u32 i;
    
    //compute csc bypass enable
    if(incs == 0x0)  //rgb 
    {
        if(outcs == 0x0) //rgb
        {
            csc_pass = 0x01;
            csc_coef_addr = (((in_csc_mode&0x3)<<7) + ((in_csc_mode&0x3)<<6)) + 0x60;
        }
        else
        {
        	//out_br_swap = 0;
            csc_pass = 0x0;
            csc_coef_addr = (((in_csc_mode&0x3)<<7) + ((in_csc_mode&0x3)<<6)) + 0x60 + 0x30;
        }
    }
    else
    {
    	//in_br_swap = 0;
        if(outcs == 0x0)
        {
            csc_pass = 0x00;
            csc_coef_addr = (((in_csc_mode&0x3)<<7) + ((in_csc_mode&0x3)<<6));
        }
        else
        {
            csc_pass = 0x01;
            csc_coef_addr = (((in_csc_mode&0x3)<<7) + ((in_csc_mode&0x3)<<6)) + 0x30;
        }
    }
    
    if(in_br_swap || out_br_swap)
   	{
   		csc_pass = 0;
   	}
   	if(!csc_pass)
    {
        for(i=0; i<4; i++)
        {
            scal_dev[sel]->csc_coef[i].dwval = csc_tab[(csc_coef_addr>>2) + i];
			      scal_dev[sel]->csc_coef[i+4 + out_br_swap * 4].dwval = csc_tab[(csc_coef_addr>>2) + i + 4 + in_br_swap * 4];
			      scal_dev[sel]->csc_coef[i+8 - out_br_swap * 4].dwval = csc_tab[(csc_coef_addr>>2) + i + 8 - in_br_swap * 4];			
        }
    }
    scal_dev[sel]->bypass.bits.csc_bypass_en = csc_pass;
    return 0;
}
//*********************************************************************************************
// description      : set scaler set output format
// parameters       :
//                 sel <scaler select>
//                 out_type <output data format>
//*********************************************************************************************** 
__s32 DE_SCAL_Set_Out_Format(__u8 sel, __scal_out_type_t *out_type)
{
	scal_dev[sel]->output_fmt.bits.byte_seq = out_type->byte_seq;
    scal_dev[sel]->output_fmt.bits.data_fmt = out_type->fmt;
    return 0;
}
//*********************************************************************************************
// description      : set scaler set output size
// parameters       :
//                 sel <scaler select>
//                 out_scan <output data scan mode>
//                 out_type <output data format>
//                 out_size <scale out size>
//*********************************************************************************************** 
__s32 DE_SCAL_Set_Out_Size(__u8 sel, __scal_scan_mod_t *out_scan, __scal_out_type_t *out_type, __scal_out_size_t *out_size)
{
    __u32 out_w1, out_h1, out_w0, out_h0;
	//sc0
    if((out_type->fmt == DE_SCAL_OUTPYUV420) || (out_type->fmt == DE_SCAL_OUTPYUV422))
    {
        out_w1 = (out_size->width+ 0x1) >> 1;
    }
    else if(out_type->fmt == DE_SCAL_OUTPYUV411)
    {
        out_w1 = (out_size->width+ 0x3) >> 2;
    }
    else
    {
        out_w1 = out_size->width;
    }

    if(out_type->fmt == DE_SCAL_OUTPYUV420)
    {
        out_h1 = (out_size->height + 0x1) >> 1;
    }
    else
    {
        out_h1 = out_size->height;
    }
	out_h0 = out_size->height;
	out_w0 = out_size->width;
	//added no-zero limited
    out_h0 = (out_h0!=0) ? out_h0 : 1;
	out_h1 = (out_h1!=0) ? out_h1 : 1;
	out_w0 = (out_w0!=0) ? out_w0 : 1;
	out_w1 = (out_w1!=0) ? out_w1 : 1;

	  scal_dev[sel]->ch0_outsize.bits.out_height = ((out_h0 + (out_scan->field & 0x1))>>out_scan->field) - 1;
    scal_dev[sel]->ch0_outsize.bits.out_width = out_w0 - 1;
    scal_dev[sel]->ch1_outsize.bits.out_height = ((out_h1 + (out_scan->field & 0x1)) >>out_scan->field) - 1;
    scal_dev[sel]->ch1_outsize.bits.out_width = out_w1 - 1;
    return 0;
}

//**********************************************************************************
// description      : scaler module start set
// parameters       :
//                 sel <scaler select>
//***********************************************************************************
__s32 DE_SCAL_Start(__u8 sel)
{
	scal_dev[sel]->frm_ctrl.bits.frm_start = 0x1;   
    return 0;
}

//**********************************************************************************
// description      : scaler configure registers set ready
// parameters       :
//                 sel <scaler select>             
//***********************************************************************************
__s32 DE_SCAL_Set_Reg_Rdy(__u8 sel)
{
    scal_dev[sel]->frm_ctrl.bits.reg_rdy_en = 0x1;   
    return 0;
}
//**********************************************************************************
// description      : scaler module reset(reset module status machine)
//***********************************************************************************
__s32 DE_SCAL_Reset(__u8 sel)
{
    scal_dev[sel]->frm_ctrl.bits.frm_start = 0x0;

    //clear wb err
    scal_dev[sel]->status.bits.wb_err_status = 0x0;
    scal_dev[sel]->status.bits.wb_err_losedata = 0x0;
    scal_dev[sel]->status.bits.wb_err_sync = 0x0;   
    return 0;
}

//**********************************************************************************
// description      : scaler module enable
//***********************************************************************************
__s32 DE_SCAL_Enable(__u8 sel)
{
	de_scal_trd_fp_en = 0;
	de_scal_trd_itl_en = 0;
    scal_dev[sel]->modl_en.bits.en = 0x1; //addr=0x0
    //scal_dev[sel]->field_ctrl.sync_edge= 0x1;   
    return 0;
}

__s32 DE_SCAL_EnableINT(__u8 sel,__u32 irqsrc)
{
	scal_dev[sel]->int_en.dwval |= irqsrc;
	
	return 0;
}

__u32 DE_SCAL_QueryINT(__u8 sel)
{	
	return scal_dev[sel]->int_status.dwval;
}

//write 1 to clear
__u32 DE_SCAL_ClearINT(__u8 sel,__u32 irqsrc)
{
		scal_dev[sel]->int_status.dwval |= DE_WB_END_IE;
	return 0;
}

//**********************************************************************************
// description      : scaler output select
// parameters       :
//                 sel <scaler select>
//                 out<0:be0; 1:be1; 2:me; 3:writeback>
//***********************************************************************************
__s32 DE_SCAL_Output_Select(__u8 sel, __u8 out)
{
    if(out == 3)//write back
    {
        scal_dev[sel]->frm_ctrl.bits.out_ctrl = 1;//disable scaler output to be/me
        scal_dev[sel]->frm_ctrl.bits.out_port_sel = 0;
    }
    else if(out < 3)
    {
        scal_dev[sel]->frm_ctrl.bits.out_ctrl = 0;//enable scaler output to be/me
        scal_dev[sel]->frm_ctrl.bits.out_port_sel = out;
    }  
    return 0;
}

//设置触发线
/*void SET_TRIG_LINE(int line)
{
 write32(DEFE_Base_Address+0X10,(read32(DEFE_Base_Address+0X10)&0xffffe000)|line);
}*/
//defe中断
/*void IRQ_DEFE_ISR(void)
{
static int defetime=0,Stime=0;	
int res=DE_SCAL_QueryINT(0);
	if(res&Wb_end)
	{	
		DE_SCAL_ClearINT(0,Wb_end);//write 1 to clear
		sysprintf("DEFE ISR wb\r\n");	
	}
	if(res&Line)
	{	
		DE_SCAL_ClearINT(0,Line);//write 1 to clear
		if((Read_time_ms()-Stime)>1000)
		{	                                                             
			sysprintf("Defe Fram %d fps\r\n",1000/(Read_time_ms()-defetime));
			Stime=Read_time_ms();	
		}	
		defetime=Read_time_ms();		
	}
}*/
//*********************************************************************************************
// function         : DE_SCAL_Set_Fb_Addr(__u8 sel, __scal_buf_addr_t *addr)
// description     : scaler change frame buffer address, only change start address parameters
// parameters    :sel <scaler select>             
//                 addr  <frame buffer address for 3 channel, 32 bit absolute address>
//***********************************************************************************************
__s32 DE_SCAL_Set_Fb_Addr(__u8 sel, __scal_buf_addr_t *addr)
{
    scal_dev[sel]->buf_addr0.dwval = addr->ch0_addr+ de_scal_ch0_offset;
    scal_dev[sel]->buf_addr1.dwval = addr->ch1_addr+ de_scal_ch1_offset;
    scal_dev[sel]->buf_addr2.dwval = addr->ch2_addr+ de_scal_ch2_offset; 
    return 0;
}
/*********************************************************************************************
DEFE初始化
*********************************************************************************************/
void Defe_Init(void)
{
	Open_Dev_Clock(DEFE_CLOCK);//开时钟	
	DE_SCAL_Set_Reg_Base(0,DEFE_Base_Address);//设置基地址	
	DE_SCAL_Reset(0);
	DE_SCAL_Enable(0);//使能
  C_BIT(DEFE_Base_Address+0X0,31);//set to 0 when using DEFE
	
}
/*********************************************************************************************
DEFE配置
配置video_yuv422_to_argb
*********************************************************************************************/
void Defe_Config_video_uvcombined_yuv422_to_argb_x(int inx,int iny,int outx,int outy,int buff_y,int buff_c)
{
	//-------------------------输入
	__scal_buf_addr_t src_buf_addr;
	__scal_scan_mod_t src_scan;
	__scal_src_size_t src_size;
	__scal_src_type_t src_type;
	//-------------------------输出
	__scal_buf_addr_t backaddr;
	__scal_scan_mod_t out_scan;
	__scal_out_type_t out_type;
	__scal_out_size_t out_size;	
	
	//-------------------------------------------输入
	src_scan.field=0;//0:frame scan; 1:field scan
	src_scan.bottom=0;//0:top field; 1:bottom field
	src_buf_addr.ch0_addr=(u32)buff_y;
	src_buf_addr.ch1_addr=(u32)buff_c;
	src_buf_addr.ch2_addr=0;
	src_size.src_width=inx;
	src_size.src_height=iny;
	src_size.x_off=0;
	src_size.y_off=0;
	src_size.scal_width=inx;
	src_size.scal_height=iny;
	src_type.sample_method=0;
	src_type.byte_seq=D0D1D2D3;
	src_type.mod=DE_SCAL_UVCOMBINED;//平面UV混合
	src_type.fmt=DE_SCAL_INYUV422;
	src_type.ps=0;
	DE_SCAL_Config_Src(0,&src_buf_addr,&src_size,&src_type,0,0);//配置输入
	
	//-------------------------------------------输出
#if (defined LCD_TYPE_TV_NTSC_720_480)||(defined LCD_TYPE_TV_PAL_720_576)
  out_scan.field=1;//0:frame scan; 1:field scan
#else
  out_scan.field=0;//0:frame scan; 1:field scan
#endif

	out_scan.bottom=0;//0:top field; 1:bottom field
	out_type.byte_seq=D3D2D1D0;//0:D0D1D2D3; 1:D3D2D1D0
	out_type.fmt=DE_SCAL_OUTI0RGB888;//0:plannar rgb888; 1: argb(byte0,byte1, byte2, byte3); 2:bgra; 4:yuv444; 5:yuv420; 6:yuv422; 7:yuv411
	out_size.width=outx;
	out_size.height=outy;//when ouput interlace enable,  the height is the 2x height of scale, for example, ouput is 480i, this value is 480
	out_size.x_off=0;
	out_size.y_off=0;
	out_size.fb_width=outx;
	out_size.fb_height=outy;				


//	backaddr.ch0_addr=0;//一个地址
//	backaddr.ch1_addr=0;
//	backaddr.ch2_addr=0;
//	DE_SCAL_Set_Writeback_Addr_ex(0,&backaddr,&out_size,&out_type);
//	DE_SCAL_Set_Writeback_Addr(0,&backaddr);
//	DE_SCAL_Writeback_Enable(0);
  DE_SCAL_Set_CSC_Coef(0,csc_bt601,csc_bt601,csc_YUV,csc_RGB,0,0);//颜色转换系数
	DE_SCAL_Set_Scaling_Factor(0, &src_scan,&src_size,&src_type,  &out_scan,&out_size,&out_type);
  DE_SCAL_Set_Init_Phase(0,&src_scan,&src_size,&src_type,  &out_scan,&out_size,&out_type,0);
  DE_SCAL_Set_Scaling_Coef(0,&src_scan,&src_size,&src_type,  &out_scan,&out_size,&out_type,0);
  DE_SCAL_Set_Scaling_Coef_for_video(0,&src_scan,&src_size,&src_type,  &out_scan,&out_size,&out_type,0);
  DE_SCAL_Set_Out_Format(0,&out_type);
	DE_SCAL_Set_Out_Size(0,&out_scan,&out_type,&out_size);              
//***********************************************************************************
//  S_BIT(DEFE_Base_Address+0X08,0);
	
  DE_SCAL_Output_Select(0,0);//out<0:debe0;  3:writeback>//输出选择
  DE_SCAL_Set_Reg_Rdy(0);//寄存器已设置好
}

/*
配置video_yuv422_to_argb
*/
extern unsigned char buff_y[3][720*576];
extern unsigned char buff_c[3][720*576];
void Defe_Config(int inmode)
{
	if(inmode==TVD_SOURCE_NTSC)
		Defe_Config_video_uvcombined_yuv422_to_argb_x(720,480,XV_W,XV_H,(int)buff_y[0],(int)buff_c[0]);
	else	if(inmode==TVD_SOURCE_PAL)
		Defe_Config_video_uvcombined_yuv422_to_argb_x(720,576,XV_W,XV_H,(int)buff_y[0],(int)buff_c[0]);	
}
void Defe_Config_x(int inmode,int w,int h)
{
	if(inmode==TVD_SOURCE_NTSC)
		Defe_Config_video_uvcombined_yuv422_to_argb_x(720,480,w,h,(int)buff_y[0],(int)buff_c[0]);
	else	if(inmode==TVD_SOURCE_PAL)
		Defe_Config_video_uvcombined_yuv422_to_argb_x(720,576,w,h,(int)buff_y[0],(int)buff_c[0]);	
}

/*********************************************************************************************
DEFE 帧 开始
*********************************************************************************************/
void Defe_Start(void)
{
	DE_SCAL_Start(0);	
}

/*********************************************************************************************
DEFE demo
*********************************************************************************************/
void Defe_Demo(void)
{

}

