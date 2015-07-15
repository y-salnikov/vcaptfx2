
#define t_width 1024
#define t_height 1024

#define BK_X0 0.115
#define BK_Y0 0.035
#define BK_X1 0.63
#define BK_Y1 0.29


#define MS_X0 0.06
#define MS_Y0 0.01
#define MS_X1 0.7
#define MS_Y1 0.3

#define ZX_X0 0.048
#define ZX_Y0 0.035
#define ZX_X1 0.34
#define ZX_Y1 0.245



typedef struct px_str_
{
	uint8_t R,G,B,A;
} px;

void init_sdl(void);
void set_resize_callback(int (*callback)(int w,int h));
extern int (*resize_callback)(int w,int h);
void show_frame(void* texture);
