int usb_init(void);
void usb_done(void);
void usb_test(void);
uint8_t usb_get_thread_state(void);
int usb_thread_function(void *not_used);
void usb_stop_thread(void);

#define USB_BUF_SIZE 256*1024
#define N_OF_TRANSFERS 5
