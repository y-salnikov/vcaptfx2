#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libconfig.h>
#include "types.h"
#include "machine.h"
#include "compat.h"
#include "default_config.h"

void machine_get_area(machine_type* mac, float* x0, float* y0, float* x1, float* y1)
{
    *x0 = mac->x0;
    *y0 = mac->y0;
    *x1 = mac->x1;
    *y1 = mac->y1;
}

void extract_color(machine_type* mac, uint8_t d, uint8_t* R, uint8_t* G, uint8_t* B)
{
    uint8_t data, i;
    data = (d ^ mac->inv_bits) & mac->pixel_bits_mask;

    for (i = 0; i < mac->colors_length; i++) {
        if (data == mac->colors[i].mask) {
            if (mac->color_mode) {
                *R = mac->colors[i].R;
                *G = mac->colors[i].G;
                *B = mac->colors[i].B;
                return;
            } else {
                *R = *G = *B = mac->colors[i].BW;
                return;
            }
        }
    }

    *R = *G = *B = 0;
}

FILE* open_cfg_file(const char* PATH, const char* FILENAME)
{
    char* full_path;
    full_path = malloc(65536);
    FILE* f;

    if (PATH) {
#ifdef WIN32
        CreateDirectory(PATH, NULL);
#elif UNIX
        mkdir(PATH, 0755);
#endif
    }

    strcpy(full_path, PATH);
    strcat(full_path, FILENAME);

    if (is_file_exist(full_path)) {
        printf("Open existing config file: %s\n", full_path);
        return fopen(full_path, "rb");
    }

    f = fopen(full_path, "wb");
    fprintf(f, "%s", config_txt);
    fclose(f);
    printf("New default config written: %s\n", full_path);
    return fopen(full_path, "rb");
}

int bin2int(const char* bin_str)
{
    int i , l, k, s;
    s = 0;
    l = strlen(bin_str);

    for (i = 0; i < l; i++) {
        k = (1 << (l - i - 1));

        if (bin_str[i] == '1') {
            s += k;
        } else if (bin_str[i] != '0') {
            return -1;
        }
    }

    return s;
}

static unsigned int to_bytes_per_ms(unsigned int samplerate)
{
    return samplerate / 1000;
}

int get_macine_config(machine_type* mac, config_setting_t* machine)
{
    int err = 0;
    int i, c, m, bw;
    const char* binstr;
    config_setting_t* colors, *color, *area;
    size_t s;
    size_t total_size;
    unsigned int timeout;

    config_setting_lookup_bool(machine, "clk_inverted",    &mac->clk_inverted);
    config_setting_lookup_int(machine,  "inv_bits",        &mac->inv_bits);
    config_setting_lookup_int(machine,  "sync_bit_mask",   &mac->sync_bit_mask);
    config_setting_lookup_int(machine,  "pixel_bits_mask", &mac->pixel_bits_mask);
    config_setting_lookup_int(machine,  "vid",             &mac->vid);
    config_setting_lookup_int(machine,  "pid",             &mac->pid);
    config_setting_lookup_int(machine,  "usb_freq",        &mac->freq);

    /*
     * The buffer should be large enough to hold 10ms of data and
     * a multiple of 512. (from sigrok)
     */

    s = 10 * to_bytes_per_ms(mac->freq);
    mac->USB_BUF_SIZE = (s + 511) & ~511;

    /* Total buffer size should be able to hold about 500ms of data. */
    mac->N_OF_TRANSFERS = (500 * to_bytes_per_ms(mac->freq)) / mac->USB_BUF_SIZE;

    if (mac->N_OF_TRANSFERS > 32) {
        mac->N_OF_TRANSFERS = 32;
    }

    total_size = mac->USB_BUF_SIZE * mac->N_OF_TRANSFERS;
    timeout = total_size / to_bytes_per_ms(mac->freq);
    mac->usb_timeout = (timeout + timeout / 4); /* Leave a headroom of 25% percent. */

    colors = config_setting_get_member(machine, "colors");

    if (colors != NULL) {
        mac->colors_length = config_setting_length(colors);
        mac->colors = calloc(mac->colors_length, sizeof(mach_colors));

        for (i = 0; i < mac->colors_length; i++) {
            color = config_setting_get_elem(colors, i);

            if (config_setting_lookup_int(color, "mask", &m) != CONFIG_TRUE) {
                if (config_setting_lookup_string(color, "mask", &binstr) != CONFIG_TRUE) {
                    err++;
                    break;
                }

                m = bin2int(binstr);

                if (m < 0) {
                    err++;
                    break;
                }
            }

            if (!((config_setting_lookup_int(color, "color", &c)) &&
                    (config_setting_lookup_int(color, "bw", &bw)))) {
                err++;
                break;
            }

            mac->colors[i].mask = ( m & 0xFF);
            mac->colors[i].R = ((c & 0xFF0000) >> 16);
            mac->colors[i].G = ((c & 0x00FF00) >> 8);
            mac->colors[i].B = ((c & 0xFF) );
            mac->colors[i].BW = ((bw & 0xFF));
        }
    }

    area = config_setting_get_member(machine, "area");

    if (area != NULL) {
        if (!((config_setting_lookup_float(area, "x0", &mac->x0)) &&
                (config_setting_lookup_float(area, "y0", &mac->y0)) &&
                (config_setting_lookup_float(area, "x1", &mac->x1)) &&
                (config_setting_lookup_float(area, "y1", &mac->y1)))) {
            err++;
        }
    }

    config_setting_lookup_int(machine, "framebuffer_size", &mac->fb_size);

    return err;
}

machine_type* machine_init(uint8_t command, const char* machine_name, const char* config_file_path)
{
    char* endptr = NULL;
    long int mach_index = 0;
    machine_type* mac;
    const char* cfg_path = NULL;
    const char* cfg_filename = NULL;
    const char* mac_name = NULL;
    FILE* config_file;
    config_t cfg;
    config_setting_t* setting, *machine, *common;
    int count, i;
    uint8_t found, err;

    err = 0;

    mac = malloc(sizeof (machine_type));
    mac->fb_size = 1024; //default value
    mac->colors_length = 0;
    mac->x0 = mac->y0 = 0.0;
    mac->x1 = mac->y1 = 1.0;
    mac->clk_inverted = mac->inv_bits = 0;
    mac->sync_bit_mask = 0x10;
    mac->pixel_bits_mask = 0x0f;
    mac->vid = 0x04b4;
    mac->pid = 0x8613;
    // mac->USB_BUF_SIZE = 16384;
    // mac->N_OF_TRANSFERS = 15;
    // mac->usb_timeout = 200;
    mac->freq = 12000000;

    if (command & COMMAND_DUMP) {
        return mac;
    }

    if (command & COMMAND_SELECT) { // -m
        mach_index = strtol(machine_name, &endptr, 0);

        if (endptr == machine_name) {
            mach_index = 0;
        }
    } else {
        mach_index = 1;
    }

    if (command & COMMAND_CONFIG) {
        cfg_path = "";
        cfg_filename = config_file_path;
    } else {
        cfg_path = get_config_file_path();
        cfg_filename = "machines.cfg";
    }

    config_file = open_cfg_file(cfg_path, cfg_filename);

    if (config_file == NULL) {
        return NULL;
    }

    config_init(&cfg);

    if (!config_read(&cfg, config_file)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return NULL;
    }

    setting = config_lookup(&cfg, "machines");

    if (setting != NULL) {
        count = config_setting_length(setting);
    } else {
        fprintf(stderr, "machines not found in config\n");
        config_destroy(&cfg);
        return NULL;
    }

    if (command & COMMAND_LIST) {
        for (i = 0; i < count; i++) {
            machine = config_setting_get_elem(setting, i);

            if ( config_setting_lookup_string(machine, "name", &mac_name) == CONFIG_TRUE) {
                printf("%d: %s\n", i + 1, mac_name);
            }
        }

        if (!(command & (COMMAND_SELECT))) {
            return NULL;
        }
    }

    if (mach_index > 0) { //select by index
        machine = config_setting_get_elem(setting, mach_index - 1);

        if (machine == NULL) {
            fprintf(stderr, "Index out of range. Use --list to check.\n");
            config_destroy(&cfg);
            return NULL;
        }
    } else {              // by name
        found = 0;

        for (i = 0; i < count; i++) {
            machine = config_setting_get_elem(setting, i);

            if (config_setting_lookup_string(machine, "name", &mac_name) == CONFIG_TRUE) {
                if (!(strcmp(mac_name, machine_name))) {
                    found++;
                    break;
                }
            }
        }

        if (found == 0) {
            fprintf(stderr, "%s not found\n", machine_name);
            return NULL;
        }
    }

    if (config_setting_lookup_string(machine, "name", &mac_name) != CONFIG_TRUE) {
        fprintf(stderr, "machine name not found in config\n");
        config_destroy(&cfg);
        return NULL;
    }

    printf("%s  selected\n", mac_name);
    mac->name = malloc(256);
    strcpy(mac->name, mac_name);

    common = config_lookup(&cfg, "common");

    if (common != NULL) {
        err += get_macine_config(mac, common);
    }

    err += get_macine_config(mac, machine);

    if (err) {
        fprintf(stderr, "Error in config\n");
        config_destroy(&cfg);
        return NULL;
    }

    mac->color_mode = 1;

    return mac;
}

void machine_done(machine_type* mac)
{
    free(mac->name);
    free(mac->colors);
    free(mac);
}
