#include <stdint.h>
#include <stdlib.h>
#include "types.h"
#include "machine.h"

const uint8_t bw_colors[16]={0x00,0x2c,0x42,0x58,0x6e,0x84,0x9a,0xb0,
							 0x00,0x50,0x71,0x8f,0x9f,0xbf,0xe3,0xff};         // magic brightness constants for my monitor


uint8_t convert_color(uint8_t c, uint8_t l, uint8_t y)
{
	if(c==0) return NULL_BRIGHTNESS;
	if(c && l && y) return FULL_BRIGHTNESS;
	if(c && l)	return NO_Y_BRIGHTNESS;
	if(c && y) return NO_L_BRIGHTNESS;
	return NO_YL_BRIGHTNESS;
}

uint8_t convert_bw(uint8_t r, uint8_t g, uint8_t b, uint8_t y)
{
	uint8_t i,c;
	i=0;
	if(b) i+=1;
	if(r) i+=2;
	if(g) i+=4;
	if(y) i+=8;
	c=bw_colors[i];
	return c;
}

void machine_get_area(machine_type *mac, float *x0, float *y0, float *x1, float *y1)
{
 if(mac->machine_idx==1)
  {
	  *x0=BK_X0;
	  *y0=BK_Y0;
	  *x1=BK_X1;
	  *y1=BK_Y1;
  }
  else
  if(mac->machine_idx==0)
  {
  	  *x0=MS_X0;
	  *y0=MS_Y0;
	  *x1=MS_X1;
	  *y1=MS_Y1;

  }
  else 
  if(mac->machine_idx==2)
  {
  	  *x0=ZX_X0;
	  *y0=ZX_Y0;
	  *x1=ZX_X1;
	  *y1=ZX_Y1;

  }
}


void extract_color(machine_type *mac, uint8_t d, uint8_t *R, uint8_t *G, uint8_t *B)
{
	uint8_t data,y;

	data=~d;
	y=data & Y_BIT;
	if(mac->machine_idx==1)
	{
		if(mac->color_mode)
		{
			*R=convert_color(!(data & R_BIT),1,1);
			*G=convert_color(!(data & G_BIT),1,1);
			*B=convert_color(!(data & B_BIT),1,1);
		}
		else
		{
			*R=convert_color(y,1,1);
			*G=convert_color(y,1,1);
			*B=convert_color(y,1,1);
		}
	}
	else
	if(mac->machine_idx==0)
	{
		if(mac->color_mode)
		{
			*R=convert_color(data & R_BIT,data & RL_BIT,y);
			*G=convert_color(data & G_BIT,data & GL_BIT,y);
			*B=convert_color(data & B_BIT,data & BL_BIT,y);
		}
		else
		{
			*R=convert_bw(data & R_BIT,data & G_BIT,data & B_BIT ,y);
			*G=convert_bw(data & R_BIT,data & G_BIT,data & B_BIT ,y);
			*B=convert_bw(data & R_BIT,data & G_BIT,data & B_BIT ,y);
		}
	}
	else
	if(mac->machine_idx==2)
	{
		if(mac->color_mode)
		{
			*R=convert_color(!(data & R_BIT),1,!y);
			*G=convert_color(!(data & G_BIT),1,!y);
			*B=convert_color(!(data & B_BIT),1,!y);
		}
		else
		{
			*R=convert_bw(!(data & R_BIT),!(data & G_BIT),!(data & B_BIT),!y);
			*G=convert_bw(!(data & R_BIT),!(data & G_BIT),!(data & B_BIT),!y);
			*B=convert_bw(!(data & R_BIT),!(data & G_BIT),!(data & B_BIT),!y);
		}
	}
}


machine_type *machine_init(uint8_t machine_idx)
{
	machine_type *mac;
	mac=malloc(sizeof (machine_type));
	mac->machine_idx=machine_idx;
	mac->color_mode=1;
	return mac;
}

void machine_done(machine_type *mac)
{
	free(mac);
}
