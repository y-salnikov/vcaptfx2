
typedef struct px_str_
{
	uint8_t R,G,B,A;
} px;

extern uint32_t resize_w,resize_h,resize_flag;

void render_init(uint8_t mach_idx);
void show_frame(void* texture);
int video_output(void);
int resizeWindow( int width, int height );
