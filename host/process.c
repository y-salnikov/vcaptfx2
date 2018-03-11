#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "render.h"
#include "process.h"
#include "machine.h"

process_context_type* process_init(machine_type* mac)
{
    process_context_type* prc;
    prc = malloc(sizeof(process_context_type));
    prc->cur_line = 0;
    prc->cur_px = 0;
    prc->machine_context = mac;
    prc->framebuf = calloc(mac->fb_size * mac->fb_size, sizeof(px));

    if (prc->framebuf == NULL) {
        fprintf(stderr, "Can't allocate %lu bytes of memory.",
                (long unsigned int)mac->fb_size * mac->fb_size * sizeof(px));
        exit(1);
    }

    return prc;
}

void process_done(process_context_type* prc)
{
    free(prc->framebuf);
    free(prc);
}

void next_pixel(process_context_type* prc, uint8_t d)
{
    uint32_t index;
    machine_type* mac;
    mac = prc->machine_context;
    index = ((prc->cur_line * mac->fb_size) + prc->cur_px);
    extract_color(mac, d, &prc->framebuf[index].R,
                  &prc->framebuf[index].G,
                  &prc->framebuf[index].B);
    prc->framebuf[index].A = 0;

    if (prc->cur_px < mac->fb_size) {
        prc->cur_px += 1;
    }
}

uint8_t v_detect(machine_type* mac, uint8_t c)
{
    static uint16_t s_counter;
    uint8_t d;
    d = (c ^ mac->inv_bits);

    if (s_counter != 65535) {
        if (d & mac->sync_bit_mask) {
            s_counter++;
        }
    }

    if (s_counter != 0) {
        if (!(d & mac->sync_bit_mask)) {
            s_counter--;
        }
    }

    if (s_counter > 74) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t h_detect(machine_type* mac, uint8_t c)
{
    static uint8_t old_c;

    if ((c ^ mac->inv_bits) & mac->sync_bit_mask) {
        old_c = 1;
    } else {
        if (old_c) {
            old_c = 0;
            return 1;
        }
    }

    return 0;
}

void parse_data(process_context_type* prc, uint8_t* buf, uint32_t length)
{
    uint32_t i;
    uint8_t c;

    for (i = 0; i < length; i++) {
        c = buf[i];

        if (h_detect(prc->machine_context, c) && (prc->cur_line < prc->machine_context->fb_size)) {
            prc->cur_px = 0;
            prc->cur_line++;
        }

        if (v_detect(prc->machine_context, c)) {
            prc->cur_line = 0;
            prc->cur_px = 0;
        }

        next_pixel(prc, c);
    }
}

