
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


#define FULL_BRIGHTNESS 0xff
#define NO_Y_BRIGHTNESS 0xB0
#define NO_L_BRIGHTNESS 0x80
#define NO_YL_BRIGHTNESS 0x50
#define NULL_BRIGHTNESS 0x00

#define R_BIT 0x01
#define G_BIT 0x02
#define B_BIT 0x04
#define RL_BIT R_BIT 
#define GL_BIT G_BIT
#define BL_BIT B_BIT 
#define Y_BIT 0x08
#define S_BIT 0x10



uint8_t convert_color(uint8_t c, uint8_t l, uint8_t y);
uint8_t convert_bw(uint8_t r, uint8_t g, uint8_t b, uint8_t y);
void machine_get_area(uint8_t mach_idx, float *x0, float *y0, float *x1, float *y1);
void extract_color(uint8_t d,uint8_t machine_idx, uint8_t c_mode, uint8_t *R, uint8_t *G, uint8_t *B);
