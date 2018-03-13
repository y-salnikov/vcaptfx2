#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include <GL/gl.h>
#include "types.h"
#include "render.h"
#include "compat.h"
#include "machine.h"
#include "process.h"
#include "no_signal.h"
#include "no_device.h"

#define WINDOW_W 640 * 2
#define WINDOW_H 288 * 3
#define RATIO ((float)WINDOW_W / (float)WINDOW_H)

void set_perspective(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int resizeWindow(render_context_type* rc, int width, int height) // {{{
{
    return 1;

    int x0, y0;
    static uint32_t w_width;
    static uint32_t w_height;

    w_width  = width;
    w_height = height;

    if (width / RATIO > height) {
        w_width  = height * RATIO;
        w_height = height;
        x0 = (width - w_width) / 2;
        y0 = 0;
    } else {
        w_width  = width;
        w_height = width / RATIO;
        x0 = 0;
        y0 = (height - w_height) / 2;
    }

    int sdl_flags = SDL_OPENGL | SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER;
    rc->sdl_surface = SDL_SetVideoMode(width, height, 0, sdl_flags);

    /* Setup our viewport. */
    glViewport(x0, y0, (GLsizei)w_width, (GLsizei)w_height);

    /* change to the projection matrix and set our viewing volume. */
    set_perspective();

    return 1;
} // }}}

void init_opengl(render_context_type* rc)
{
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlsetvideomode.html
    rc->sdl_surface = SDL_SetVideoMode(WINDOW_W, WINDOW_H, 0, SDL_OPENGL | SDL_RESIZABLE);
    if (rc->sdl_surface == NULL) {
        printf("Can't set OpenGL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_WM_SetCaption("video capture", NULL);
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glClearStencil(0);
    glClearDepth(1.0f);
    glGenTextures(1, &rc->fb_texture);
    set_perspective();
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

    machine_get_area(rc->machine_context, &rc->tx0, &rc->ty0, &rc->tx1, &rc->ty1);
    init_opengl(rc);
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
    x0 = (fb_width  * (rc->machine_context->x0 + rc->machine_context->x1) / 2) - (img_width / 2);
    y0 = (fb_height * (rc->machine_context->y0 + rc->machine_context->y1) / 2) - (img_height / 2);

    int x, y, pixel_idx;
    px* fb_pixel;
    for (y = 0; y < img_height; y++)
        for (x = 0; x < img_width; x++) {
            pixel_idx = (img_width * y + x) * 4;
            fb_pixel = &rc->process_context->framebuf[(fb_width * (y + y0)) + x0 + x];
            fb_pixel->R = img_pixels[pixel_idx];
            fb_pixel->G = img_pixels[pixel_idx + 1];
            fb_pixel->B = img_pixels[pixel_idx + 2];
            fb_pixel->A = img_pixels[pixel_idx + 3];
        }
} // }}}

void update_texture(render_context_type* rc )
{
    if (rc->no_signal_flag)
        draw_centered_image(rc, no_signal_img.width, no_signal_img.height, no_signal_img.pixel_data);
    if (rc->no_device_flag)
        draw_centered_image(rc, no_device_img.width, no_device_img.height, no_device_img.pixel_data);
    glBindTexture(GL_TEXTURE_2D, rc->fb_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    int fb_width  = rc->process_context->machine_context->fb_width;
    int fb_height = rc->process_context->machine_context->fb_height;
    int sb_width  = fb_width * 2;

    px* framebuf  = rc->process_context->framebuf;
    px* scalerbuf = rc->process_context->scalerbuf;
    // const px black_pixel = { .R = 0, .G = 0, .B = 0, .A = 0 };
    px current_pixel;

    int x, y, sb_x;
    int fb_line, sb_line0, sb_line1, sb_line2;
    for (y = 0; y < fb_height; y++)
    {
        fb_line = fb_width * y;
        sb_line0 = y * 3 * sb_width;
        sb_line1 = sb_line0 + sb_width;
        sb_line2 = sb_line1 + sb_width;
        for (x = 0; x < fb_width; x++)
        {
            current_pixel = framebuf[fb_line + x];
            sb_x = x * 2;
            scalerbuf[sb_line0 + sb_x + 0] = current_pixel;
            scalerbuf[sb_line0 + sb_x + 1] = current_pixel;
            // scalerbuf[sb_line1 + sb_x + 0] = current_pixel;
            // scalerbuf[sb_line1 + sb_x + 1] = current_pixel;
            // scalerbuf[sb_line2 + sb_x + 0] = black_pixel;
            // scalerbuf[sb_line2 + sb_x + 1] = black_pixel;
        }
        memcpy((void*)&scalerbuf[sb_line1], (void*)&scalerbuf[sb_line0], sizeof(px) * sb_width);
        memset((void*)&scalerbuf[sb_line2], 0, sizeof(px) * sb_width);
    }

    glTexImage2D(
            GL_TEXTURE_2D, // target
            0, // level
            GL_RGBA, // internalFormat
            fb_width * 2, // width
            fb_height * 3, // height
            0, // border
            GL_RGBA, // format
            GL_UNSIGNED_BYTE, // type
            scalerbuf // data
            );
}

void show_frame(render_context_type* rc) //{{{
{
    glLoadIdentity();
    // очистка буферов
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    update_texture(rc);

    glBegin(GL_QUADS);
      glTexCoord2f(rc->tx0, rc->ty1);
      glVertex2f(0.0, 0.0);

      glTexCoord2f(rc->tx1, rc->ty1);
      glVertex2f(1.0, 0.0);

      glTexCoord2f(rc->tx1, rc->ty0);
      glVertex2f(1.0, 1.0);

      glTexCoord2f(rc->tx0, rc->ty0);
      glVertex2f(0.0, 1.0);
    glEnd();

    glFlush();
    SDL_GL_SwapBuffers();
} // }}}

int video_output(render_context_type* rc)
{
    show_frame(rc);
    SLEEP(10);
    return 0;
}
