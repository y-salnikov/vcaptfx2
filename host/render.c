#include <stdlib.h>
#include <stdint.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include <GL/gl.h>
#include "render.h"

SDL_Surface *screen;
SDL_Thread *video;

extern uint8_t video_stop;
px *framebuf;
uint32_t w_width=800;
uint32_t w_height=600;
float ratio=800.0/600.0;
uint32_t t_width=1024;
uint32_t t_height=1024;
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
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
//	gluPerspective(40,(double)(w_width)/(double)(w_height),1,50);
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


int resizeWindow( int width, int height )
{
    int x0,y0;
    /* Height / width ration */
//    GLfloat ratio;

    /* Protect against a divide by zero */
//    if ( height == 0 )
//    height = 1;
    w_width=width;
    w_height=height;
    if(width/ratio > height)
    {
        w_width=height*ratio;
        w_height=height;
        x0=(width-w_width)/2;
        y0=0;
    } else
    {
        w_width=width;
        w_height=width/ratio;
        x0=0;
        y0=(height-w_height)/2;
    }
//    ratio = ( GLfloat )width / ( GLfloat )height;
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
    if (NULL == (screen = SDL_SetVideoMode(w_width, w_height, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER )))
    {
        printf("Can't set OpenGL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    } 
    glClearColor(0.0,0.0,0.0,0.0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_WM_SetCaption("video capture",NULL);
    glViewport(0,0,w_width,w_height);
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LEQUAL);
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

//  init_opengl ();
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
//	char str[256];
//	static uint32_t fc=0;
    glLoadIdentity();

//    gluLookAt(0,0,-3,  0,0,z_dst,  0,1,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//    update_texture();
//    bind_fire_texture();
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
//    sprintf(str,"FRAME %d",fc++);
//    SDL_WM_SetCaption(str,NULL);
}

int video_output(void *notused)
{
    static int init_flag;
    
    while(!video_stop)
    {
        if(init_flag==0)
        {
              init_opengl ();
              init_flag=1;
        }
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
}
