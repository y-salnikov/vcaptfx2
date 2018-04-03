/*
 * Based on
 * cycfx2dev.cc - Cypress FX2 device class: low-level routines..
 *.
 * Copyright (c) 2006--2009 by Wolfgang Wieser ] wwieser (a) gmx <*> de [.
*/


#include <stdint.h>
#include "libusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "command.h"
#include "compat.h"
#include "types.h"
#include "process.h"
#include "usb.h"
#include "profile.h"

#define MAX_EMPTY_TRANSFERS 64

LIBUSB_CALL void callbackUSBTransferComplete(struct libusb_transfer* xfr);

int WriteRAM(usb_transfer_context_type* utc, size_t addr, const unsigned char* data, size_t nbytes)
{
    int n_errors = 0;
    const size_t chunk_size = 16;
    const unsigned char* d = data;
    const unsigned char* dend = data + nbytes;

    while (d < dend) {
        size_t bs = dend - d;

        if (bs > chunk_size) {
            bs = chunk_size;
        }

        size_t dl_addr = addr + (d - data);
        int rv = libusb_control_transfer(utc->device_h, 0x40, 0xa0,
                                         /*addr=*/dl_addr, 0,
                                         /*buf=*/(unsigned char*)d,
                                         /*size=*/bs,
                                         /*timeout=*/1000/*msec*/);

        if (rv < 0) {
            fprintf(stderr, "Error: Writing %lu bytes at 0x%X\n", (long unsigned int)bs, (unsigned int)dl_addr);
            ++n_errors;
        }

        d += bs;
    }

    return (n_errors);
}

int ReadRAM(usb_transfer_context_type* utc, size_t addr, const unsigned char* data, size_t nbytes)
{
    int n_errors = 0;
    size_t bs;
    size_t rd_addr;
    int rv;
    const size_t chunk_size = 16;
    const unsigned char* d = data;
    const unsigned char* dend = data + nbytes;

    while (d < dend) {
        bs = dend - d;

        if (bs > chunk_size) {
            bs = chunk_size;
        }

        rd_addr = addr + (d - data);
        rv = libusb_control_transfer(utc->device_h, 0xc0, 0xa0,
                                     /*addr=*/rd_addr, 0,
                                     /*buf=*/(unsigned char*)d,
                                     /*size=*/bs,
                                     /*timeout=*/1000/*msec*/);

        if (rv < 0) {
            fprintf(stderr, "Error: Reading %lu bytes at 0x%X\n", (long unsigned int)bs, (unsigned int)rd_addr);
            ++n_errors;
        }

        d += bs;
    }

    return (n_errors);
}

int FX2Reset(usb_transfer_context_type* utc, uint8_t running)
{
    // Reset is accomplished by writing a 1 to address 0xE600.
    // Start running by writing a 0 to that address.
    const size_t reset_addr = 0xe600;
    unsigned char val = running ? 0 : 1;

    return (WriteRAM(utc, reset_addr, &val, 1));
}

int ProgramIHexLine(usb_transfer_context_type* utc, const char* buf,    const char* path, int line)
{
    const char* s;
    unsigned int nbytes = 0, addr = 0, type = 0;
    unsigned int file_cksum;

    s = buf;

    if (*s != ':') {
        fprintf(stderr, "%s:%d: format violation (1)\n", path, line);
        return (1);
    }

    ++s;

    if (sscanf(s, "%02x%04x%02x", &nbytes, &addr, &type) != 3) {
        fprintf(stderr, "%s:%d: format violation (2)\n", path, line);
        return (1);
    }

    s += 8;

    if (type == 0) {
        //printf("  Writing nbytes=%d at addr=0x%04x\n",nbytes,addr);
        if (!(nbytes < 256)) {
            fprintf(stderr, "%s:%d: format violation (2)\n", path, line);
            return (1);
        }

        unsigned char* data;
        unsigned char cksum;
        unsigned int i, d;

        data = malloc(nbytes);
        cksum = nbytes + addr + (addr >> 8) + type;

        for (i = 0; i < nbytes; i++) {
            d = 0;

            if (sscanf(s, "%02x", &d) != 1) {
                fprintf(stderr, "%s:%d: format violation (3)\n", path, line);
                free(data);
                return (1);
            }

            s += 2;
            data[i] = d;
            cksum += d;
        }

        file_cksum = 0;

        if (sscanf(s, "%02x", &file_cksum) != 1) {
            fprintf(stderr, "%s:%d: format violation (4)\n", path, line);
            free(data);
            return (1);
        }

        if ((cksum + file_cksum) & 0xff) {
            fprintf(stderr, "%s:%d: checksum mismatch (%u/%u)\n",
                    path, line, cksum, file_cksum);
            free(data);
            return (1);
        }

        if (WriteRAM(utc, addr, data, nbytes)) {
            free(data);
            return (1);
        }

        free(data);
    } else if (type == 1) {
        // EOF marker. Oh well, trust it.
        return (-1);
    } else {
        fprintf(stderr, "%s:%d: Unknown entry type %d\n", path, line, type);
        return (1);
    }

    return (0);
}

usb_transfer_context_type*  usb_init(const char** firmware, void* proc_cont)
{

    usb_transfer_context_type* utc;

    if (libusb_init(NULL)) {
        fprintf(stderr, "libusb Init error\n");
        return NULL;
    }

    utc = malloc(sizeof(usb_transfer_context_type));
    utc->active_transfers = 0;
    utc->usb_transfer_cb_served = 0;
    utc->usb_stop_flag = 2; // 0 -running, 1-stoppes, 2-not inited, 3- no device
    utc->process_context = proc_cont;
    utc->tmp_buffer = malloc(utc->process_context->machine_context->USB_BUF_SIZE);
    utc->device_h = libusb_open_device_with_vid_pid(NULL, utc->process_context->machine_context->vid,
                                                    utc->process_context->machine_context->pid);

    if (utc->device_h == NULL) {
        //      fprintf(stderr,"No device found or device already configured.\n");
    } else {
        printf("Transfering firmware\n");

        int err = libusb_set_configuration(utc->device_h, 1);

        if (err) {
            fprintf(stderr, "Can't set device configuration\n");
            free(utc);
            return NULL;
        }

        err = libusb_claim_interface(utc->device_h, 0);

        if (err) {
            fprintf(stderr, "Can't claim device interface\n");
            free(utc);
            return NULL;
        }

        utc->device = libusb_get_device(utc->device_h);
        err = libusb_get_active_config_descriptor(utc->device, &utc->conf);

        if (err) {
            fprintf(stderr, "Can't get device configuration descriptor\n");
            free(utc);
            return NULL;
        }

        FX2Reset(utc, 0);
        const char** ss = firmware;
        err = 0;
        int line = 0;

        const char* s;
        do {
            s = *ss++;

            if (s != NULL) {
                //printf("%s\n",s);
                err = ProgramIHexLine(utc, s, "vcapt_firmvare", line);
                line++;
            }
        } while (s);

        FX2Reset(utc, 1);
        libusb_free_config_descriptor   (utc->conf);
        err = libusb_release_interface(utc->device_h, 0);

        if (err) {
            fprintf(stderr, "Can't release device interface err=%d\n", err);
        }

        libusb_close(utc->device_h);
        libusb_exit(NULL);
        //wait 3 sec for renumeration
        SLEEP(3000);

    }

    if (libusb_init(NULL)) {
        fprintf(stderr, "libusb Init error\n");
        free(utc);
        return NULL;
    }

    utc->device_h = libusb_open_device_with_vid_pid(NULL, 0xffff, 0x2048);

    if (utc->device_h == NULL) {
        //      fprintf(stderr,"No reconfigured device found\n");
        free(utc);
        return NULL;
    }

    int err = libusb_set_configuration(utc->device_h, 1);

    if (err) {
        fprintf(stderr, "Can't set device configuration\n");
        free(utc);
        return NULL;
    }

    err = libusb_claim_interface(utc->device_h, 0);

    if (err) {
        fprintf(stderr, "Can't claim device interface\n");
        free(utc);
        return NULL;
    }

    utc->device = libusb_get_device(utc->device_h);
    err = libusb_get_active_config_descriptor(utc->device, &utc->conf);

    if (err) {
        fprintf(stderr, "Can't get device configuration descriptor\n");
        free(utc);
        return NULL;
    }

    utc->endpoint = utc->conf->interface[0].altsetting[0].endpoint[0].bEndpointAddress;

    return utc;
}

void usb_done(usb_transfer_context_type* utc)
{
    int err;

    libusb_free_config_descriptor(utc->conf);

    err = libusb_release_interface(utc->device_h, 0);

    if (err) {
        fprintf(stderr, "Can't release device interface err=%d\n", err);
    }

    libusb_close(utc->device_h);
    libusb_exit(NULL);
    free(utc->control_buffer);
    free(utc);
}

void usb_send_start_cmd(usb_transfer_context_type* utc)
{
    uint8_t flg;
    struct libusb_transfer* xfr;

    flg = CMD_START_FLAGS_SAMPLE_8BIT;

    if (utc->process_context->machine_context->clk_inverted) {
        flg |= CMD_START_FLAGS_INV_CLK;
    }

    utc->control_buffer = malloc(64);
    xfr = libusb_alloc_transfer(0);
    libusb_fill_control_setup(utc->control_buffer, 0x40, CMD_START, 0, 0, 3);
    utc->control_buffer[8] = flg;
    utc->control_buffer[9] = 0;
    utc->control_buffer[10] = 0;

    libusb_fill_control_transfer(xfr, utc->device_h, utc->control_buffer, callbackUSBTransferComplete,
                                 NULL, 1000);

    if (libusb_submit_transfer(xfr) < 0) {
        // Error
        libusb_free_transfer(xfr);
        free(utc->control_buffer);
        fprintf(stderr, "USB submit control transfer error\n");
    }
}

void usb_start_transfer (usb_transfer_context_type* utc)
{
    uint8_t i;

    if (utc == NULL) {
        return;
    }

    usb_send_start_cmd(utc);

    for (i = 0; i < utc->process_context->machine_context->N_OF_TRANSFERS; i++) {
        uint8_t* usb_buf = malloc(utc->process_context->machine_context->USB_BUF_SIZE);
        struct libusb_transfer* xfr = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(xfr, utc->device_h, utc->endpoint, usb_buf,
                                  utc->process_context->machine_context->USB_BUF_SIZE, callbackUSBTransferComplete, utc,
                                  utc->process_context->machine_context->usb_timeout );

        if (libusb_submit_transfer(xfr) < 0) {
            // Error
            libusb_free_transfer(xfr);
            free(usb_buf);
            fprintf(stderr, "USB submit transfer %d error\n", i);
        } else {
            utc->active_transfers++;
        }
    }


}

LIBUSB_CALL void callbackUSBTransferComplete(struct libusb_transfer* xfr)
{
    uint8_t err = 0;
    uint8_t fatal = 0;
    usb_transfer_context_type* utc;
    static int et_count;
    size_t lngth;

    if (xfr->user_data == NULL) { // control transfer
        et_count = 0;

        if (xfr->status != LIBUSB_TRANSFER_COMPLETED) {
            fprintf(stderr, "USB controll transfer error\n");
        }
    } else {
        utc = xfr->user_data;

        switch (xfr->status) {
            case LIBUSB_TRANSFER_NO_DEVICE:
                fatal = 1;
                break;

            case LIBUSB_TRANSFER_COMPLETED:
            case LIBUSB_TRANSFER_TIMED_OUT:
                break;

            default:
                err = 1;
                break;
        }

        if (!fatal) {
            if (xfr->actual_length == 0 || err) {
                et_count++;

                if (et_count >= MAX_EMPTY_TRANSFERS) {
                    //                  fprintf(stderr,"%d empty transfers\n",MAX_EMPTY_TRANSFERS);
                    fatal = 1;
                } else {
                    if (libusb_submit_transfer(xfr) < 0) {
                        // Error
                        libusb_free_transfer(xfr);
                        free(xfr->buffer);
                        fprintf(stderr, "USB resubmit transfer error\n");
                        utc->active_transfers--;
                        err = 1;
                    }
                }
            } else {
                et_count = 0;
                lngth = xfr->actual_length;
                memcpy(utc->tmp_buffer, xfr->buffer, lngth);

                if (libusb_submit_transfer(xfr) < 0) {
                    // Error
                    libusb_free_transfer(xfr);
                    free(xfr->buffer);
                    fprintf(stderr, "USB resubmit transfer error\n");
                    utc->active_transfers--;
                    err = 1;
                }

                parse_data(utc->process_context, utc->tmp_buffer, lngth);
            }
        }

        if (fatal) {
            libusb_free_transfer(xfr);
            free(xfr->buffer);
            utc->active_transfers--;

            if (utc->active_transfers == 0) {
                utc->usb_stop_flag = 1;
            }

            return;
        }

        utc->usb_transfer_cb_served = 1;
    }
}

void usb_poll(void) // need to be periodicaly called
{
    struct timeval tv;
    tv.tv_sec = tv.tv_usec = 0;

    if (libusb_handle_events_timeout(NULL, &tv) != LIBUSB_SUCCESS) {
        fprintf(stderr, "USB error event\n");
    }
}

int usb_thread_function(void* utc_ptr)
{
    usb_transfer_context_type* utc;
    utc = utc_ptr;

    if (utc == NULL) {
        return 0;
    }

    usb_start_transfer(utc);
    utc->usb_stop_flag = 0;

    while (utc->usb_stop_flag < 2) {
        usb_poll();
        SLEEP(1);
    }

    return 0;
}

uint8_t usb_get_thread_state(usb_transfer_context_type* utc)
{
    if (utc == NULL) {
        return 3;
    }

    if (libusb_claim_interface(utc->device_h, 0)) {
        free(utc);
        utc = NULL;
        return 3;
    }

    return utc->usb_stop_flag;
}

void usb_stop_thread(usb_transfer_context_type* utc)
{
    utc->usb_stop_flag = 1;
}
