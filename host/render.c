#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "SDL.h"
#include "types.h"
#include "render.h"
#include "compat.h"
#include "machine.h"
#include "process.h"
#include "no_signal.h"
#include "no_device.h"

#define WINDOW_W 640 * 2
#define WINDOW_H 288 * 3

int resizeWindow(render_context_type* rc, int width, int height)
{
    return 1;
}

void init_SDL_surface(render_context_type* rc)
{
    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlsetvideomode.html
    rc->sdl_surface = SDL_SetVideoMode(WINDOW_W, WINDOW_H, 32, SDL_HWSURFACE);
    if (rc->sdl_surface == NULL) {
        printf("Can't init SDL surface: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    SDL_WM_SetCaption("vcaptfx2: MC0511", NULL);

    const SDL_VideoInfo *info = SDL_GetVideoInfo();

    printf("Is it possible to create hardware surfaces? %d\n",          info->hw_available);
    printf("Is there a window manager available? %d\n",                 info->wm_available);
    printf("Are hardware to hardware blits accelerated? %d\n",          info->blit_hw);
    printf("Are hardware to hardware colorkey blits accelerated? %d\n", info->blit_hw_CC);
    printf("Are hardware to hardware alpha blits accelerated? %d\n",    info->blit_hw_A);
    printf("Are software to hardware blits accelerated? %d\n",          info->blit_sw);
    printf("Are software to hardware colorkey blits accelerated? %d\n", info->blit_sw_CC);
    printf("Are software to hardware alpha blits accelerated? %d\n",    info->blit_sw_A);
    printf("Are color fills accelerated? %d\n",                         info->blit_fill);
    printf("Total amount of video memory in Kilobytes %d\n",            info->video_mem);

    printf("w %d\n", rc->sdl_surface->w);
    printf("h %d\n", rc->sdl_surface->h);
    printf("pitch %d\n", rc->sdl_surface->pitch);
    printf("BitsPerPixel %d\n", rc->sdl_surface->format->BitsPerPixel);
    printf("BytesPerPixel %d\n", rc->sdl_surface->format->BytesPerPixel);
    printf("refcount %d\n", rc->sdl_surface->refcount);
}

render_context_type* render_init(void* machine_context, void* process_context)
{
    render_context_type* rc;
    rc = malloc(sizeof(render_context_type));
    rc->machine_context = machine_context;
    rc->process_context = process_context;
    rc->no_signal_flag = 0;
    rc->no_device_flag = 0;

    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf ("Unable to init SDL: %s\n", SDL_GetError ());
        exit (1);
    }

    init_SDL_surface(rc);
    return rc;
}

void render_done(render_context_type* rc)
{
    free(rc);
}

void draw_centered_image(render_context_type* rc, int img_width, int img_height,
                         const unsigned char* img_pixels) // {{{
{
    int x0, y0;
    int fb_width  = rc->process_context->machine_context->fb_width;
    int fb_height = rc->process_context->machine_context->fb_height;
    x0 = (fb_width  / 2) - (img_width  / 2) + (3 - rand() % 6);
    y0 = (fb_height / 2) - (img_height / 2) + (3 - rand() % 6);

    int x, y, fb_line, img_line, pixel_idx;
    px* fb_pixel;
    for (y = 0; y < img_height; y++)
    {
        img_line = img_width * y;
        fb_line = (fb_width * (y + y0));
        for (x = 0; x < img_width; x++) {
            pixel_idx = (img_line + x) * 4;
            fb_pixel = &rc->process_context->framebuf[fb_line + x0 + x];
            fb_pixel->R = img_pixels[pixel_idx];
            fb_pixel->G = img_pixels[pixel_idx + 1];
            fb_pixel->B = img_pixels[pixel_idx + 2];
            fb_pixel->A = img_pixels[pixel_idx + 3];
        }
    }
} // }}}

void update_sdl_surface(render_context_type* rc )
{
    //SDL_LockSurface(rc->sdl_surface);
    if (rc->no_signal_flag)
        draw_centered_image(rc, no_signal_img.width, no_signal_img.height, no_signal_img.pixel_data);
    if (rc->no_device_flag)
        draw_centered_image(rc, no_device_img.width, no_device_img.height, no_device_img.pixel_data);

    int fb_width  = rc->process_context->machine_context->fb_width;
    int fb_height = rc->process_context->machine_context->fb_height;

    int sb_width  = rc->process_context->machine_context->sb_width;

    px* framebuf  = rc->process_context->framebuf;
    px* scalerbuf = rc->sdl_surface->pixels;
    px current_pixel;

    int x, y, sb_x;
    int fb_line, sb_line0, sb_line1;
    for (y = 0; y < fb_height; y++)
    {
        fb_line = fb_width * y;
        sb_line0 = y * 3 * sb_width;
        sb_line1 = sb_line0 + sb_width;
        for (x = 0; x < fb_width; x++)
        {
            current_pixel.R = framebuf[fb_line + x].B;
            current_pixel.G = framebuf[fb_line + x].R;
            current_pixel.B = framebuf[fb_line + x].G;
            current_pixel.A = 0; // framebuf[fb_line + x].A;
            sb_x = x * 2;
            scalerbuf[sb_line0 + sb_x + 0] = current_pixel;
            scalerbuf[sb_line0 + sb_x + 1] = current_pixel;
        }
        memcpy((void*)&scalerbuf[sb_line1], (void*)&scalerbuf[sb_line0], rc->sdl_surface->pitch);
    }
    //SDL_UnlockSurface(rc->sdl_surface);
}

int video_output(render_context_type* rc)
{
    update_sdl_surface(rc);
    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlflip.html
    // SDL_Flip(rc->sdl_surface);

    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlupdaterect.html
    SDL_UpdateRect(rc->sdl_surface, 0, 0, 0, 0);

    SLEEP(10);
    return 0;
}
