#define fb_width 1024
#define fb_height 1024

process_context_type* process_init(void *mac);
void parse_data(process_context_type *prc, uint8_t *buf, uint32_t length);
void process_done(process_context_type *prc);


