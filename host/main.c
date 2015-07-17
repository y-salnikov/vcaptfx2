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






int main(int argc, char **argv)
{
	uint8_t stop=0;
	uint8_t machine=0;
	uint8_t scr_l_e=1;
	event_type ev=NOTHING;
	process_context_type *pcont;
	usb_transfer_context_type *utc;
	machine_type *mac;
	render_context_type *rc;
	
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
	mac=machine_init(machine);
    pcont=process_init(mac);
    utc=usb_init(vcapt_firmware,pcont);
    if(utc==NULL)
	{
		exit(1);
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

