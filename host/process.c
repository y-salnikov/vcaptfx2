#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "process.h"
#include "machine.h"


extern uint8_t machine;
extern uint8_t color_mode;
uint16_t cur_line=0;
uint16_t cur_px=0;

extern px *framebuf;


int process_init()
{
	framebuf=calloc(t_width*t_height,sizeof(px));
	if (framebuf==NULL)
	{
		fprintf(stderr,"Can't allocate %lu bytes of memory.",(long unsigned int)fb_width*fb_height*sizeof(px));
		exit(1);
	}
	
	
	return 0;
}

void process_done(void)
{
	free(framebuf);
	
}





void next_pixel(uint8_t d)
{
	uint32_t index;
	index=((cur_line*fb_width)+cur_px);
	extract_color(d,machine,color_mode,&framebuf[index].R,&framebuf[index].G,&framebuf[index].B);
	framebuf[index].A=0;
	if(cur_px<fb_width) cur_px+=1;
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



