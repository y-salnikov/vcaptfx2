#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "usb.h"



int argc=1;

unsigned int chunksize;
uint8_t *buf;
uint8_t stop=0;
uint8_t machine=0; //ms0511
uint8_t color_mode=1;
uint8_t scr_l_e=1;
uint8_t video_stop=0;
uint64_t data_size=0;
uint8_t	data_ready=0;

void init_()
{
	int err;
	err=usb_init();
	if(err) exit(err);

}

void done_(void)
{
	
}

void data_ready_cb(void *usb_buffer,uint64_t length)
{
	memcpy(buf,usb_buffer,length);
	data_size=length;
	data_ready++;
}


int main(int argc, char **argv)
{
	init_();
    int readed;
    int ret;
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
    buf=malloc(USB_BUF_SIZE);
    usb_start_transfer();
    if(buf==NULL) fprintf(stderr,"Can't allocate %d bytes of memory\n",USB_BUF_SIZE);
    else
    {
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
				if(data_ready==1)
				{
					parse_data(buf,data_size);
					data_ready--;
				}
				else
				if(data_ready>1)
				{
					fprintf(stderr,"Buffer overrun!\n");
					data_ready=0;
				}
				
            }
            video_stop=1;
            while(video_stop!=2)
            {
				usleep(1000000);
				printf("Waiting video stream to close\n");
			}
        free(buf);
        done();
    }
    
	return 0;
}
