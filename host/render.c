#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_thread.h"
#include <GL/gl.h>
#include "render.h"

SDL_Surface *screen;
SDL_Thread *video;

extern uint8_t video_stop;
px *framebuf;
#define WINDOW_W 800
#define WINDOW_H 600
#define RATIO (800.0/600.0)


GLuint fb_texture;
extern uint8_t machine;
float tx0,ty0,tx1,ty1;
extern uint32_t resize_w,resize_h,resize_flag;
int dummy(int w,int h);
int (*resize_callback)(int w,int h)=dummy;

int video_output(void *notused);

void set_resize_callback(int (*callback)(int w,int h))
{
    resize_callback=callback;
}




void set_perspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


int resizeWindow( int width, int height )
{
    int x0,y0;
	static uint32_t w_width;
	static uint32_t w_height;

    w_width=width;
    w_height=height;
    if(width/RATIO > height)
    {
        w_width=height*RATIO;
        w_height=height;
        x0=(width-w_width)/2;
        y0=0;
    } else
    {
        w_width=width;
        w_height=width/RATIO;
        x0=0;
        y0=(height-w_height)/2;
    }

    screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER );
    /* Setup our viewport. */
    glViewport( x0, y0, ( GLsizei )w_width, ( GLsizei )w_height );
    
    /* change to the projection matrix and set our viewing volume. */
    set_perspective();

    return 1;
}





void init_opengl(void)
{
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    if (NULL == (screen = SDL_SetVideoMode(WINDOW_W, WINDOW_H, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER )))
    {
        printf("Can't set OpenGL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    } 
    glClearColor(0.0,0.0,0.0,0.0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_WM_SetCaption("video capture",NULL);
    glViewport(0,0,WINDOW_W,WINDOW_H);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glClearStencil(0);     
    glClearDepth(1.0f);
    glGenTextures(1,&fb_texture);
	set_perspective();
    set_resize_callback(resizeWindow);
}



void init_sdl(void)
{
if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
      printf ("Unable to init SDL: %s\n", SDL_GetError ());
      exit (1);
    }

  if(machine==1)
  {
	  tx0=BK_X0;
	  ty0=BK_Y0;
	  tx1=BK_X1;
	  ty1=BK_Y1;
  }
  else
  if(machine==0)
  {
  	  tx0=MS_X0;
	  ty0=MS_Y0;
	  tx1=MS_X1;
	  ty1=MS_Y1;

  }
  else 
  if(machine==2)
  {
  	  tx0=ZX_X0;
	  ty0=ZX_Y0;
	  tx1=ZX_X1;
	  ty1=ZX_Y1;

  }
  video_stop=0;
  video=SDL_CreateThread(video_output,NULL);
}

int dummy(int w,int h)
{
    return 0;
}

void update_texture( void* texture)
{
    
    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t_width, t_height, 0, GL_RGBA,GL_UNSIGNED_BYTE,texture);
}

void show_frame(void* texture)
{

    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

     update_texture(texture);

    glBegin(GL_QUADS);

        glTexCoord2f(tx0,ty1);
        glVertex2f(0.0,0.0);
        
        glTexCoord2f(tx1,ty1);
        glVertex2f(1.0,0.0);
        
        glTexCoord2f(tx1,ty0);
        glVertex2f(1.0,1.0);
        
        glTexCoord2f(tx0,ty0);
        glVertex2f(0.0,1.0);
    glEnd();
      
    glFlush();
    SDL_GL_SwapBuffers();
}

int video_output(void *notused)
{
     
    init_opengl ();
    
    while(!video_stop)
    {
        show_frame(framebuf);
        if(resize_flag)
        {
            resizeWindow(resize_w,resize_h);
            resize_flag=0;
        }
        usleep(1000);
    }
    video_stop=2;
    SDL_Quit();
    return 0;
}
