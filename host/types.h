#include "libusb.h"
#include "SDL.h"

typedef struct px {
    uint8_t R, G, B, A;
} px;

typedef struct {
    uint8_t R, G, B;
    uint8_t BW;
} mach_colors;

typedef struct machine_struct {
    int frame_width;
    int frame_height;
    int h_counter_shift;
    int v_counter_shift;
    int fullscreen_width;
    int fullscreen_height;
    uint8_t color_mode;
    char* name;
    int clk_inverted;
    int inv_bits;
    int sync_bit_mask;
    int pixel_bits_mask;
    mach_colors colors[256];
    int vid;
    int pid;
    int USB_BUF_SIZE;
    int N_OF_TRANSFERS;
    int usb_timeout;
    int freq;
} machine_type;

typedef struct process_context_struct {
    px* framebuf;
    px* framebuf_position;
    int cur_line;
    int cur_px;
    machine_type* machine_context;
} process_context_type;


typedef struct render_context_struct {
    SDL_Surface* sdl_surface;
    int viewport_width;
    int viewport_height;
    void (*render_function)(render_context_type);
    int is_fullscreen;
    uint8_t no_signal_flag;
    uint8_t no_device_flag;
    process_context_type* process_context;
} render_context_type;

typedef struct usb_transfer_context_struct {
    struct libusb_config_descriptor* conf;
    libusb_device_handle* device_h;
    libusb_device* device;
    uint8_t active_transfers;
    unsigned char endpoint;
    uint8_t usb_transfer_cb_served;
    uint8_t usb_stop_flag;
    uint8_t* control_buffer;
    process_context_type* process_context;
    uint8_t* tmp_buffer;
} usb_transfer_context_type;

