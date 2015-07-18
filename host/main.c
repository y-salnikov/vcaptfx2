#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "libusb.h"
#include "types.h"
#include "usb.h"
#include "compat.h"
#include "render.h"
#include "process.h"
#include "SDL.h"
#include "SDL_thread.h"
#include "machine.h"

extern const char *vcapt_firmware[];
uint8_t usb_stop=0;

typedef enum EVENTS {EVENT_QUIT, EVENT_RESIZE, ESC_PRESSED, SCL_PRESSED, NOTHING}  event_type;




event_type read_events(render_context_type *rc)
{
	SDL_Event event;
	if (SDL_PollEvent (&event))
    {
      if (event.type == SDL_QUIT)
		{
			return EVENT_QUIT;
		}
		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
		    {
				return ESC_PRESSED;
		    }
			if (event.key.keysym.sym == SDLK_SCROLLOCK)
			{
				return SCL_PRESSED;
			}
		}
		if (event.type == SDL_VIDEORESIZE)
		{
			resizeWindow(rc,event.resize.w,event.resize.h);
			return EVENT_RESIZE;
		} 

    }
  return NOTHING;
}



void print_help(void)
{
	printf("Usege:  vcaptfx2 --list | -l  : List all machines\n");
	printf("                 --machine | -m <index> or <name> [-c PATH_TO_CFG]: Start capture from  chosen machine.\n\n");
	printf("default config file will be writen at  %s\n", get_config_file_path());
	printf("First machine profile if no arguments passed\n");
	exit(0);
}


int main(int argc, char **argv)
{
	uint8_t stop=0;
	uint8_t scr_l_e=1;
	event_type ev=NOTHING;
	process_context_type *pcont;
	usb_transfer_context_type *utc;
	machine_type *mac;
	render_context_type *rc;
	uint8_t command=0;
	uint8_t test=0;
	char* mach_name=NULL;
	char* cfg_file=NULL;
    if(argc==2)
    {
        if((strcmp(argv[1],"-l")==0) || (strcmp(argv[1],"--list")==0))
        {
            command=1;
        } else
        {
			if(strcmp(argv[1],"-t")==0)
			{
				test=1;
				command=2;
				mach_name="1";
			}
			else	print_help();
		}
        
    }
    else
    if(argc==3)
    {
		if((strcmp(argv[1],"-m")==0) || (strcmp(argv[1],"--machine")==0))
		{
			mach_name=argv[2];
			command=2;
		}
	}
	else
	if(argc==5)
    {
		if(((strcmp(argv[1],"-m")==0) || (strcmp(argv[1],"--machine")==0)) && (strcmp(argv[3],"-f")==0))
		{
			mach_name=argv[2];
			cfg_file=argv[4];
			command=3;
		}
	}
	if(argc==1)
	{
		command=2;
		mach_name="1";
	}
	
	mac=machine_init(command,mach_name,cfg_file);
	if (mac==NULL) print_help();
    pcont=process_init(mac);
    utc=usb_init(vcapt_firmware,pcont);
    if(utc==NULL)
	{
		exit(1);
	}
	if(test)
	{
		usb_test(utc);
		exit(0);
	}
    rc=render_init(mac,pcont);
    
    SDL_CreateThread(usb_thread_function,utc);
    while(usb_get_thread_state(utc)==2)
    {
		SLEEP(1);						//wait USB thread to start
	}
            while(!stop)
            {
				ev=read_events(rc);
                if (ev==ESC_PRESSED || ev==EVENT_QUIT) stop=1;
                if(ev==SCL_PRESSED && scr_l_e)
                    {
                        mac->color_mode=1-mac->color_mode;
                        scr_l_e=0;
                    }
                if(ev!=SCL_PRESSED) scr_l_e=1;  // SCROLL_LOCK key release
                if(usb_get_thread_state(utc))
                {
					stop=1;
					fprintf(stderr,"USB thread stopped. 0x%X\n",usb_get_thread_state(utc));
				}
				video_output(rc);
            }
            usb_stop_thread(utc);


        usb_done(utc);
        process_done(pcont);
        render_done(rc);
    
    
	return 0;
}

