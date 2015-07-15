#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "usb.h"
#include "video.h"
#include "keys.h"



uint8_t stop=0;
uint8_t machine=0; //ms0511
uint8_t color_mode=1;
uint8_t scr_l_e=1;
uint8_t video_stop=0;



void init_()
{
	int err;
	err=usb_init();
	if(err) exit(err);

}

void done_(void)
{
	video_done();
	usb_done();
}

void data_ready_cb(void *usb_buffer,uint64_t length)
{
	static uint8_t	data_ready;
	data_ready++;
	if(data_ready==1)
	{
		parse_data(usb_buffer,length);
	}
	else
	if(data_ready>1)
	{
		fprintf(stderr,"Buffer overrun!\n");
	}
	data_ready=0;
}


int main(int argc, char **argv)
{
	init_();
    if(argc==2)
    {
        if(strcmp(argv[1],"-bk")==0)
        {
            machine=1;
            printf("БК-0010-01 mode\n");
        } else
        if(strcmp(argv[1],"-zx")==0)
        {
			machine=2;
			printf("ZX Spectrum mode\n");
		}
    }
    video_init();
    usb_start_transfer();
            begin_listen_keys();
            while(!stop)
            {
                if (esc_pressed()) stop=1;
                if(scl_pressed() && scr_l_e)
                    {
                        color_mode=1-color_mode;
                        scr_l_e=0;
                    }
                if(scl_pressed()==0) scr_l_e=1;
                usb_poll();
            }
            video_stop=1;
            while(video_stop!=2)
            {
				usleep(500000);
				printf("Waiting video thread to close\n");
			}
        done_();
    
    
	return 0;
}
