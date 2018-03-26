#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libconfig.h>
#include "types.h"
#include "machine.h"
#include "compat.h"
#include "default_config.h"

void extract_color(machine_type* mac, uint8_t d, uint8_t* R, uint8_t* G, uint8_t* B)
{
    uint8_t mask;
    mask = (d ^ mac->inv_bits) & mac->pixel_bits_mask;

    *R = mac->colors[mask].R;
    *G = mac->colors[mask].G;
    *B = mac->colors[mask].B;
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
    int i, l, s;
    s = 0;
    l = strlen(bin_str);

    for (i = 0; i < l; i++) {
        int k = (1 << (l - i - 1));

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

int get_machine_config(machine_type* mac, config_setting_t* machine)
{
    int err = 0;
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

    if (mac->N_OF_TRANSFERS > 32)
        mac->N_OF_TRANSFERS = 32;

    total_size = mac->USB_BUF_SIZE * mac->N_OF_TRANSFERS;
    timeout = total_size / to_bytes_per_ms(mac->freq);
    mac->usb_timeout = (timeout + timeout / 4); /* Leave a headroom of 25% percent. */

    config_setting_lookup_int(machine, "frame_width", &mac->frame_width);
    config_setting_lookup_int(machine, "frame_height", &mac->frame_height);

    config_setting_lookup_int(machine, "h_counter_shift", &mac->h_counter_shift);
    config_setting_lookup_int(machine, "v_counter_shift", &mac->v_counter_shift);

    config_setting_lookup_int(machine, "fullscreen_width", &mac->fullscreen_width);
    config_setting_lookup_int(machine, "fullscreen_height", &mac->fullscreen_height);
    return err;
}

// colors mask  Y   RL   GL   BL   R    G    B
//           0x40 0x20 0x10 0x08 0x04 0x02 0x01
int swap_rg(int i)
{
    int r, g, gt, c;
    r = i & 0x04;
    g = i & 0x02;
    gt = r >> 1;
    r  = g << 1;
    g = gt;
    c = r + g + (i & 0x79);
    return c;
}

void colors_bw(match_color colors[])
{
    for(int i = 0; i < 128; i++)
    {
        int c = swap_rg(i);
        colors[i].R = colors[i].G = colors[i].B = (c & 0x07) * 0x24;
    }
}

void colors_16(match_color colors[], int grb)
{
    match_color color;
    for(int i = 0; i < 128; i++)
    {
        color.R = color.G = color.B = 0;

        if ((i & 0x04) && !(i & 0x40)) { color.R = 0xB0; }
        if ((i & 0x02) && !(i & 0x40)) { color.G = 0xB0; }
        if ((i & 0x01) && !(i & 0x40)) { color.B = 0xB0; }

        if ((i & 0x44) == 0x44) { color.R = 0xFF; }
        if ((i & 0x42) == 0x42) { color.G = 0xFF; }
        if ((i & 0x41) == 0x41) { color.B = 0xFF; }

        if (grb)
            colors[swap_rg(i)] = color;
        else
            colors[i] = color;
    }
}

void colors_128(match_color colors[], int grb)
{
    const uint8_t C = 0x80; // 0x80
    const uint8_t L = 0x2B; // 0x2B;
    const uint8_t Y = 0x54; // 0x54;
    match_color color;

    for(int i = 0; i < 128; i++)
    {
        color.R = color.G = color.B = 0;

        if (i & 0x04) { color.R += C; }
        if (i & 0x02) { color.G += C; }
        if (i & 0x01) { color.B += C; }

        if (i & 0x20) { color.R += L; }
        if (i & 0x10) { color.G += L; }
        if (i & 0x08) { color.B += L; }

        if ((i & 0x40) && (i & 0x24)) { color.R += Y; }
        if ((i & 0x40) && (i & 0x12)) { color.G += Y; }
        if ((i & 0x40) && (i & 0x09)) { color.B += Y; }

        if (grb)
            colors[swap_rg(i)] = color;
        else
            colors[i] = color;
    }
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

    uint8_t err = 0;

    mac = malloc(sizeof (machine_type));
    mac->vid = 0x04B4;
    mac->pid = 0x8613;
    mac->freq = 12500000;
    // mac->USB_BUF_SIZE = 16384;
    // mac->N_OF_TRANSFERS = 15;
    // mac->usb_timeout = 200;
    mac->frame_width  = 640;
    mac->frame_height = 288;
    mac->h_counter_shift = -73;
    mac->v_counter_shift = -17;
    mac->clk_inverted = 1;
    mac->inv_bits = 0x47;
    mac->sync_bit_mask = 0x80;
    mac->pixel_bits_mask = 0x7F;
    colors_16(mac->colors, 0);

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
        free(mac);
        return NULL;
    }

    config_init(&cfg);

    if (!config_read(&cfg, config_file)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        free(mac);
        return NULL;
    }

    setting = config_lookup(&cfg, "machines");

    if (setting != NULL) {
        count = config_setting_length(setting);
    } else {
        fprintf(stderr, "machines not found in config\n");
        config_destroy(&cfg);
        free(mac);
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
            free(mac);
            return NULL;
        }
    }

    if (mach_index > 0) { //select by index
        machine = config_setting_get_elem(setting, mach_index - 1);

        if (machine == NULL) {
            fprintf(stderr, "Index out of range. Use --list to check.\n");
            config_destroy(&cfg);
            free(mac);
            return NULL;
        }
    } else {              // by name
        int found = 0;

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
            free(mac);
            return NULL;
        }
    }

    if (config_setting_lookup_string(machine, "name", &mac_name) != CONFIG_TRUE) {
        fprintf(stderr, "machine name not found in config\n");
        config_destroy(&cfg);
        free(mac);
        return NULL;
    }

    printf("%s  selected\n", mac_name);
    mac->name = malloc(256);
    strcpy(mac->name, mac_name);

    common = config_lookup(&cfg, "common");

    if (common != NULL) {
        err += get_machine_config(mac, common);
    }

    err += get_machine_config(mac, machine);

    if (err) {
        fprintf(stderr, "Error in config\n");
        config_destroy(&cfg);
        return NULL;
    }

    return mac;
}

void machine_done(machine_type* mac)
{
    free(mac->name);
    free(mac);
}
