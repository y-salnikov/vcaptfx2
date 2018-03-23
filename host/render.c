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

int resizeWindow(render_context_type* rc, int width, int height)
{
    return 1;
}

void init_SDL_surface(render_context_type* rc)
{
    int w, h;
    uint32_t sdl_flags = SDL_HWSURFACE;
    if (rc->is_fullscreen)
    {
        sdl_flags |= SDL_FULLSCREEN;
        w = rc->process_context->machine_context->fullscreen_width;
        h = rc->process_context->machine_context->fullscreen_height;
    } else {
        w = rc->viewport_width;
        h = rc->viewport_height;
    }
    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlsetvideomode.html

    rc->sdl_surface = SDL_SetVideoMode(w, h, 32, sdl_flags);
    if (rc->sdl_surface == NULL) {
        printf("Can't init SDL surface: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    SDL_WM_SetCaption("vcaptfx2: MC0511", NULL);
}

render_context_type* render_init(void* machine_context, void* process_context)
{
    render_context_type* rc;
    rc = malloc(sizeof(render_context_type));
    rc->process_context = process_context;
    rc->no_signal_flag = 0;
    rc->no_device_flag = 0;
    rc->viewport_width  = rc->process_context->machine_context->frame_width  * 2;
    rc->viewport_height = rc->process_context->machine_context->frame_height * 3;
    rc->render_function = update_sdl_surface_2x;
    rc->is_fullscreen = 0;

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
    int frame_width  = rc->process_context->machine_context->frame_width;
    int frame_height = rc->process_context->machine_context->frame_height;
    x0 = (frame_width  / 2) - (img_width  / 2) + (3 - rand() % 6);
    y0 = (frame_height / 2) - (img_height / 2) + (3 - rand() % 6);

    int x, y, pixel_idx;
    px* fb_pixel;
    for (y = 0; y < img_height; y++)
    {
        int img_line = img_width * y;
        int fb_line = (frame_width * (y + y0));
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

void update_sdl_surface_2x(render_context_type* rc)
{
    int frame_width  = rc->process_context->machine_context->frame_width;
    int frame_height = rc->process_context->machine_context->frame_height;

    px* framebuf = rc->process_context->framebuf;
    px* viewport = rc->sdl_surface->pixels;
    px current_pixel;

    // add a shift to center vertically
    viewport += rc->sdl_surface->w * ((rc->sdl_surface->h - rc->viewport_height) / 2);
    // add a shift to center horizontally
    viewport += (rc->sdl_surface->w - rc->viewport_width) / 2;
    int x, y;
    //SDL_LockSurface(rc->sdl_surface);
    for (y = 0; y < frame_height; y++)
    {
        px* vp_line0_start = viewport + y * 3 * rc->sdl_surface->w;
        px* vp_line0 = vp_line0_start;
        px* vp_line1 = vp_line0 + rc->sdl_surface->w;
        for (x = 0; x < frame_width; x++)
        {
            current_pixel = *(framebuf++);
            *(vp_line0++) = current_pixel;
            *(vp_line0++) = current_pixel;
        }
        memcpy((void*)vp_line1, (void*)vp_line0_start, rc->viewport_width * sizeof(px));
    }
    //SDL_UnlockSurface(rc->sdl_surface);
}

px average_rgb(px a, px b)
{
    px avg;
    avg.R = (a.R + b.R) / 2;
    avg.G = (a.G + b.G) / 2;
    avg.B = (a.B + b.B) / 2;
    avg.A = (a.A + b.A) / 2;
    return avg;
}

#define AVERAGERGB(a, b) ( ((((unsigned long)a) & 0xfefefeffUL) + (((unsigned long)b) & 0xfefefeffUL)) >> 1 )
void update_sdl_surface_74x(render_context_type* rc)
{
    int frame_width  = rc->process_context->machine_context->frame_width;
    int frame_height = rc->process_context->machine_context->frame_height;

    px* framebuf  = rc->process_context->framebuf;
    px* viewport = rc->sdl_surface->pixels;
    px pix1, pix2, pix3, pix4;

    // add a shift to center vertically
    viewport += rc->sdl_surface->w * ((rc->sdl_surface->h - rc->viewport_height) / 2);
    // add a shift to center horizontally
    viewport += (rc->sdl_surface->w - rc->viewport_width) / 2;
    int x, y;
    //SDL_LockSurface(rc->sdl_surface);
    for (y = 0; y < frame_height; y++)
    {
        px* vp_line0_start = viewport + y * 3 * rc->sdl_surface->w;
        px* vp_line0 = vp_line0_start;
        px* vp_line1 = vp_line0 + rc->sdl_surface->w;

        for (x = 0; x < frame_width / 4; x++)
        {
            pix1 = *(framebuf++);
            pix2 = *(framebuf++);
            pix3 = *(framebuf++);
            pix4 = *(framebuf++);

            *(vp_line0++) = pix1;
            *(vp_line0++) = average_rgb(pix1, pix2);
            *(vp_line0++) = pix2;
            *(vp_line0++) = average_rgb(pix2, pix3);
            *(vp_line0++) = pix3;
            *(vp_line0++) = average_rgb(pix3, pix4);
            *(vp_line0++) = pix4;
        }
        memcpy((void*)vp_line1, (void*)vp_line0_start, rc->viewport_width * sizeof(px));
    }
    //SDL_UnlockSurface(rc->sdl_surface);
}

int video_output(render_context_type* rc)
{
    if (rc->no_signal_flag)
        draw_centered_image(rc, no_signal_img.width, no_signal_img.height, no_signal_img.pixel_data);
    if (rc->no_device_flag)
        draw_centered_image(rc, no_device_img.width, no_device_img.height, no_device_img.pixel_data);

    rc->render_function(rc);

    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlflip.html
    // SDL_Flip(rc->sdl_surface);

    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlupdaterect.html
    int x = (rc->sdl_surface->w - rc->viewport_width)  / 2;
    int y = (rc->sdl_surface->h - rc->viewport_height) / 2;
    SDL_UpdateRect(rc->sdl_surface, x, y, rc->viewport_width, rc->viewport_height);

    SLEEP(15);
    return 0;
}
