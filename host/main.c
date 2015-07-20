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
#include <stdlib.h>
#include <getopt.h>

extern const char *vcapt_firmware[];
uint8_t usb_stop=0;
static int verbose_flag;

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
	printf("Usege:  vcaptfx2  [--list | -l] [--dump | -d] \n");
	printf("                  [--machine | -m <name or index>] [--config | -c <file>]\n\n");
	printf("--list    : List all machines\n");
	printf("--dump    : dump 1MB of raw data to file usb_log.dat for debuging.\n");
	printf("--machine : Select machine profile and start capture video.\n");
	printf("--config  : Config file to use. Will be created if not exist.\n");
	printf("          : default config file will be writen at  %s\n", get_config_file_path());
	printf("First machine profile in config will be used if no --machine or -m options passed.\n");
	exit(0);
}


int main(int argc, char **argv)
{
	uint8_t stop=0;
	uint8_t scr_l_e=1;
	int c;
	event_type ev=NOTHING;
	process_context_type *pcont;
	usb_transfer_context_type *utc;
	machine_type *mac;
	render_context_type *rc;
	uint8_t command=0;
	uint8_t test=0;
	char* mach_name=NULL;
	char* cfg_file=NULL;

    while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose", no_argument,       &verbose_flag, 1},
          {"brief",   no_argument,       &verbose_flag, 0},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"list",     no_argument,       0, 'l'},
          {"dump",  no_argument,       0, 'd'},
          {"help",  no_argument,       0, 'h'},
          {"machine",  required_argument, 0, 'm'},
          {"config",  required_argument, 0, 'c'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "ldhm:c:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'l':
          command|=COMMAND_LIST;
          break;

        case 'd':
          test=1;
          command|=COMMAND_DUMP;
          break;

        case 'h':
          print_help();
          break;

        case 'c':
			command|=COMMAND_CONFIG;
			cfg_file=optarg;
          break;

        case 'm':
          command|=COMMAND_SELECT;
			mach_name=optarg;
          break;
        
        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }

	mac=machine_init(command,mach_name,cfg_file);
	if (mac==NULL) exit(3);
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

