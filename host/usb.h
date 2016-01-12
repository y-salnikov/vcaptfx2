
usb_transfer_context_type*  usb_init(const char **firmware, void	*proc_cont);
void usb_done(usb_transfer_context_type *utc);
void usb_test(usb_transfer_context_type *utc);
uint8_t usb_get_thread_state(usb_transfer_context_type *utc);
int usb_thread_function(void *utc_ptr);
void usb_stop_thread(usb_transfer_context_type *utc);




