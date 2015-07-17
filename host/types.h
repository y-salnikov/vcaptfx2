#include "libusb.h"

typedef struct
{
	uint8_t R,G,B,A;
} px;


typedef struct machine_struct
{
	uint8_t machine_idx;
	uint8_t color_mode;
	
	
}machine_type;




typedef struct process_context_struct
{
	px *framebuf;
	uint16_t cur_line;
	uint16_t cur_px;
	machine_type *machine_context;
	
} process_context_type;


typedef struct render_context_struct
{
	void *sdl_surface;
	uint fb_texture;
	float tx0,ty0,tx1,ty1;
	machine_type *machine_context;
	process_context_type *process_context;
	
}render_context_type;

typedef struct usb_transfer_context_struct
{
	struct libusb_config_descriptor *conf;
	libusb_device_handle *device_h;
	libusb_device *device;
	uint8_t active_transfers;
	unsigned char endpoint;
	uint8_t usb_transfer_cb_served;
	uint8_t usb_stop_flag;
	process_context_type	*process_context;
	
}usb_transfer_context_type;


