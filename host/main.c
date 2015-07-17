#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "usb.h"
#include "compat.h"
#include "render.h"
#include "process.h"
#include "SDL.h"
#include "SDL_thread.h"



uint8_t machine=0; //ms0511
uint8_t color_mode=1;
uint8_t usb_stop=0;

typedef enum EVENTS {EVENT_QUIT, EVENT_RESIZE, ESC_PRESSED, SCL_PRESSED, NOTHING}  event_type;



void init_all(uint8_t mach_idx)
{
	int err;
	process_init();
	err=usb_init();
	if(err)
	{
		fprintf(stderr,"libusb error\n");
		exit(err);
	}
	render_init(mach_idx);

	

}

void done_all(void)
{
	usb_done();
	process_done();
	
}


event_type read_events(void)
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
			resizeWindow(event.resize.w,event.resize.h);
			return EVENT_RESIZE;
		} 

    }
  return NOTHING;
}






int main(int argc, char **argv)
{
	uint8_t stop=0;
	uint8_t scr_l_e=1;
	event_type ev=NOTHING;

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
    init_all(machine);
    
    SDL_CreateThread(usb_thread_function,NULL);
    while(usb_get_thread_state()==2)
    {
		SLEEP(1);						//wait USB thread to start
	}
            while(!stop)
            {
				ev=read_events();
                if (ev==ESC_PRESSED || ev==EVENT_QUIT) stop=1;
                if(ev==SCL_PRESSED && scr_l_e)
                    {
                        color_mode=1-color_mode;
                        scr_l_e=0;
                    }
                if(ev!=SCL_PRESSED) scr_l_e=1;  // SCROLL_LOCK key release
                if(usb_get_thread_state())
                {
					stop=1;
					fprintf(stderr,"USB thread stopped. 0x%X\n",usb_get_thread_state());
				}
				video_output();
            }
            usb_stop_thread();
            
        done_all();
    
    
	return 0;
}

