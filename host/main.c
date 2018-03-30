#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "libusb.h"
#include "types.h"
#include "colors.h"
#include "usb.h"
#include "compat.h"
#include "render.h"
#include "process.h"
#include "SDL.h"
#include "SDL_thread.h"
#include "machine.h"
#include "Vcaptfx2Config.h"

extern const char* vcapt_firmware[];
uint8_t usb_stop = 0;
static int verbose_flag;

typedef enum EVENTS {EVENT_QUIT, EVENT_RESIZE, ESC_PRESSED,
                     ONE_PRESSED, TWO_PRESSED, A_PRESSED, Q_PRESSED, W_PRESSED, S_PRESSED, E_PRESSED, D_PRESSED,
                     FULLSCREEN_PRESSED, INTERLACED_PRESSED, NOTHING} event_type;

event_type read_events(render_context_type* rc)
{
    SDL_Event event;

    if (SDL_PollEvent (&event)) {
        if (event.type == SDL_QUIT) {
            return EVENT_QUIT;
        }

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) { return ESC_PRESSED; }

            if (event.key.keysym.sym == SDLK_1) { return ONE_PRESSED; }
            if (event.key.keysym.sym == SDLK_2) { return TWO_PRESSED; }
            if (event.key.keysym.sym == SDLK_q) { return Q_PRESSED; }
            if (event.key.keysym.sym == SDLK_a) { return A_PRESSED; }
            if (event.key.keysym.sym == SDLK_w) { return W_PRESSED; }
            if (event.key.keysym.sym == SDLK_s) { return S_PRESSED; }
            if (event.key.keysym.sym == SDLK_e) { return E_PRESSED; }
            if (event.key.keysym.sym == SDLK_d) { return D_PRESSED; }

            if (event.key.keysym.sym == SDLK_f) { return FULLSCREEN_PRESSED; }
            if (event.key.keysym.sym == SDLK_i) { return INTERLACED_PRESSED; }
        }

        if (event.type == SDL_VIDEORESIZE) {
            resizeWindow(rc, event.resize.w, event.resize.h);
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
    printf("Version: %d.%1.1f\n", VERSION_MAJOR, VERSION_MINOR);
    exit(0);
}


int main(int argc, char** argv)
{
    uint8_t stop = 0;
    event_type ev = NOTHING;
    process_context_type* pcont;
    usb_transfer_context_type* utc;
    machine_type* mac;
    render_context_type* rc;
    uint8_t command = 0;
    uint8_t test = 0;
    char* mach_name = NULL;
    char* cfg_file = NULL;
    uint8_t signal_present = 100;

    while (1) {
        static struct option long_options[] = {
            /* These options set a flag. */
            {"verbose", no_argument,       &verbose_flag, 1},
            {"brief",   no_argument,       &verbose_flag, 0},
            /* These options don’t set a flag.
               We distinguish them by their indices. */
            {"list",    no_argument,       0, 'l'},
            {"dump",    no_argument,       0, 'd'},
            {"help",    no_argument,       0, 'h'},
            {"machine", required_argument, 0, 'm'},
            {"config",  required_argument, 0, 'c'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "ldhm:c:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:

                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0) {
                    break;
                }

                printf ("option %s", long_options[option_index].name);

                if (optarg) {
                    printf (" with arg %s", optarg);
                }

                printf ("\n");
                break;

            case 'l':
                command |= COMMAND_LIST;
                break;

            case 'd':
                test = 1;
                command |= COMMAND_DUMP;
                break;

            case 'h':
                print_help();
                break;

            case 'c':
                command |= COMMAND_CONFIG;
                cfg_file = optarg;
                break;

            case 'm':
                command |= COMMAND_SELECT;
                mach_name = optarg;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort ();
        }
    }

    mac = machine_init(command, mach_name, cfg_file);

    if (mac == NULL) {
        exit(3);
    }

    pcont = process_init(mac);
    utc = usb_init(vcapt_firmware, pcont);

    if (test) {
        //usb_test(utc);
        exit(0);
    }

    rc = render_init(mac, pcont);

    SDL_CreateThread(usb_thread_function, utc);

    while (usb_get_thread_state(utc) == 2) {
        SLEEP(1); //wait USB thread to start
    }

    int base_color_idx = 0;

    while (!stop) {
        ev = read_events(rc);

        if (ev == ESC_PRESSED || ev == EVENT_QUIT) {
            stop = 1;
        }

        if (ev == ONE_PRESSED) {
            rc->viewport_width  = rc->process_context->machine_context->frame_width  * 7 / 4;
            rc->viewport_height = rc->process_context->machine_context->frame_height * 3;
            rc->render_function = update_sdl_surface_74x;
            init_SDL_surface(rc);
        }

        if (ev == TWO_PRESSED) {
            rc->viewport_width  = rc->process_context->machine_context->frame_width  * 2;
            rc->viewport_height = rc->process_context->machine_context->frame_height * 3;
            rc->render_function = update_sdl_surface_2x;
            init_SDL_surface(rc);
        }

        if (ev == FULLSCREEN_PRESSED) {
            rc->fullscreen = 1 - rc->fullscreen;
            init_SDL_surface(rc);
        }

        if (ev == INTERLACED_PRESSED) {
            rc->interlaced = 1 - rc->interlaced;
            SDL_FillRect(rc->sdl_surface, NULL, 0);
        }

        if (ev == Q_PRESSED) {
            colors_bw(mac->colors, base_color_idx);
        }

        if (ev == A_PRESSED) {
            base_color_idx = (base_color_idx + 1) % 4;
            colors_bw(mac->colors, base_color_idx);
        }

        if (ev == W_PRESSED)
            colors_16(mac->colors, 0);

        if (ev == S_PRESSED)
            colors_16(mac->colors, 1);

        if (ev == E_PRESSED)
            colors_128(mac->colors, 0);

        if (ev == D_PRESSED)
            colors_128(mac->colors, 1);

        if (usb_get_thread_state(utc) == 3) {
            rc->no_device_flag = 1;
            utc = usb_init(vcapt_firmware, pcont); //atempt to connect to device

            if (utc) {
                rc->no_device_flag = 0;
                rc->no_signal_flag = 1;
                signal_present = 90;
            }
        } else if (usb_get_thread_state(utc)) {
            //usb thread stopped
            rc->no_signal_flag = 1;
            signal_present = 0;
            SDL_CreateThread(usb_thread_function, utc); //atempting to restart usb_transfer
        } else {
            if (signal_present > 99) {
                rc->no_signal_flag = 0;
            } else {
                signal_present++;
            }
        }

        video_output(rc);
    }

    return 0;
    usb_stop_thread(utc);

    usb_done(utc);
    process_done(pcont);
    render_done(rc);
}

