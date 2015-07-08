#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "render.h"

#define FULL_BRIGHTNESS 0xff
#define NO_Y_BRIGHTNESS 0xB0
#define NO_L_BRIGHTNESS 0x80
#define NO_YL_BRIGHTNESS 0x50
#define NULL_BRIGHTNESS 0x00


#define pix_in_line 800

#define R_BIT 0x01
#define G_BIT 0x02
#define B_BIT 0x04
#define RL_BIT R_BIT 
#define GL_BIT G_BIT
#define BL_BIT B_BIT 
#define Y_BIT 0x08
#define S_BIT 0x10



const uint8_t bw_colors[16]={0x00,0x2c,0x42,0x58,0x6e,0x84,0x9a,0xb0,
							 0x00,0x50,0x71,0x8f,0x9f,0xbf,0xe3,0xff};         // magic brightness constants for my monitor


extern uint8_t machine;
extern uint8_t color_mode;
uint16_t cur_line=0;
uint16_t cur_px=0;

extern px *framebuf;
uint8_t *frame_data;

int video_init()
{
	framebuf=calloc(t_width*t_height,sizeof(px));
	if (framebuf==NULL)
	{
		fprintf(stderr,"Can't allocate %lu bytes of memory.",fb_width*fb_height*sizeof(px));
		exit(1);
	}
	frame_data=malloc(512*1024);
	if (frame_data==NULL)
	{
		fprintf(stderr,"Can't allocate %lu bytes of memory.",fb_width*fb_height*sizeof(px));
		exit(1);
	}
	init_sdl();
	
	return 0;
}

uint8_t convert_color(c,l,y)
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
	if(b) i+=1;
	if(r) i+=2;
	if(g) i+=4;
	if(y) i+=8;
	c=bw_colors[i];
	return c;
}


void next_pixel(uint8_t d)
{
	uint8_t y,data;
	uint32_t index;
	data=~d;
	index=((cur_line*fb_width)+cur_px);
	y=data & Y_BIT;
	if(machine==1)
	{
		if(color_mode)
		{
			framebuf[index].R=convert_color(!(data & R_BIT),1,1);
			framebuf[index].G=convert_color(!(data & G_BIT),1,1);
			framebuf[index].B=convert_color(!(data & B_BIT),1,1);
		}
		else
		{
			framebuf[index].R=convert_color(y,1,1);
			framebuf[index].G=convert_color(y,1,1);
			framebuf[index].B=convert_color(y,1,1);
		}
	}
	else
	if(machine==0)
	{
		if(color_mode)
		{
			framebuf[index].R=convert_color(data & R_BIT,data & RL_BIT,y);
			framebuf[index].G=convert_color(data & G_BIT,data & GL_BIT,y);
			framebuf[index].B=convert_color(data & B_BIT,data & BL_BIT,y);
			framebuf[index].A=0;
		}
		else
		{
			framebuf[index].R=convert_bw(data & R_BIT,data & G_BIT,data & B_BIT ,y);
			framebuf[index].G=convert_bw(data & R_BIT,data & G_BIT,data & B_BIT ,y);
			framebuf[index].B=convert_bw(data & R_BIT,data & G_BIT,data & B_BIT ,y);
			framebuf[index].A=0;
		}
	}
	else
	if(machine==2)
	{
		if(color_mode)
		{
			framebuf[index].R=convert_color(!(data & R_BIT),1,!y);
			framebuf[index].G=convert_color(!(data & G_BIT),1,!y);
			framebuf[index].B=convert_color(!(data & B_BIT),1,!y);
			framebuf[index].A=0;
		}
		else
		{
			framebuf[index].R=convert_bw(!(data & R_BIT),!(data & G_BIT),!(data & B_BIT),!y);
			framebuf[index].G=convert_bw(!(data & R_BIT),!(data & G_BIT),!(data & B_BIT),!y);
			framebuf[index].B=convert_bw(!(data & R_BIT),!(data & G_BIT),!(data & B_BIT),!y);
			framebuf[index].A=0;
		}
	}

	if(cur_px<fb_width) cur_px+=1;
//	printf("%d\n",cur_line);
}



uint8_t v_detect(uint8_t c)
{
	static uint16_t s_counter;
	if(s_counter != 65535)
	{
		if(c & S_BIT) s_counter++;
	}
	if(s_counter!=0)
	{
		if(!(c & S_BIT)) s_counter--;
	}
	if(s_counter>74) return 1;
	else return 0;
}

uint8_t h_detect(uint8_t c)
{
	static uint8_t old_c;
	if(c & S_BIT) old_c=1;
	else
	{
		if(old_c)
		{
			old_c=0;
			return 1;
		}
	}
	return 0;
	
}



void parse_data(uint8_t *buf, uint32_t length)
{
	uint32_t i;
	uint8_t c;
	for(i=0;i<length;i++)
	{
		c=buf[i];

		if (h_detect(c) && (cur_line<fb_height))
		{
			cur_px=0;
			cur_line++;
		}
		if (v_detect(c))
		{
			cur_line=0;
			cur_px=0;
		}

		next_pixel(c);
	}
}



