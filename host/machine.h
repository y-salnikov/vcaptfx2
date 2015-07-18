

void machine_get_area(machine_type *mac, float *x0, float *y0, float *x1, float *y1);
void extract_color(machine_type *mac, uint8_t d, uint8_t *R, uint8_t *G, uint8_t *B);
machine_type *machine_init(uint8_t command, const char* machine_name, const char* config_file_path);
void machine_done(machine_type *mac);
