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
#include <time.h>
#include "command.h"
#include "usb.h"
#include "main.h"

extern const char *vcapt_firmware[];
extern uint8_t stop;
struct libusb_config_descriptor *conf;
libusb_device_handle *device_h;
libusb_device *device;

unsigned char ep2in;
unsigned char ep0;





void callbackUSBTransferComplete(struct libusb_transfer *xfr);

int WriteRAM(size_t addr,const unsigned char *data, size_t nbytes)
{
	int n_errors=0;
	size_t bs;
	size_t dl_addr;
	int rv;
	const size_t chunk_size=16;
	const unsigned char *d=data;
	const unsigned char *dend=data+nbytes;
	while(d<dend)
	{
		bs=dend-d;
		if(bs>chunk_size)  bs=chunk_size;
		dl_addr=addr+(d-data);
		rv=libusb_control_transfer(device_h,0x40,0xa0,
			/*addr=*/dl_addr,0,
			/*buf=*/(char*)d,/*size=*/bs,
			/*timeout=*/1000/*msec*/);
		if(rv<0)
		{  fprintf(stderr,"Error: Writing %lu bytes at 0x%X\n",bs,dl_addr);  ++n_errors;  }
		d+=bs;
	}
	return(n_errors);
}


int ReadRAM(size_t addr,const unsigned char *data, size_t nbytes)
{
	int n_errors=0;
	size_t bs;
	size_t rd_addr;
	int rv;
	const size_t chunk_size=16;
	const unsigned char *d=data;
	const unsigned char *dend=data+nbytes;
	while(d<dend)
	{
		bs=dend-d;
		if(bs>chunk_size)  bs=chunk_size;
		rd_addr=addr+(d-data);
		rv=libusb_control_transfer(device_h,0xc0,0xa0,
			/*addr=*/rd_addr,0,
			/*buf=*/(char*)d,/*size=*/bs,
			/*timeout=*/1000/*msec*/);
		if(rv<0)
		{  fprintf(stderr,"Error: Reading %lu bytes at 0x%X\n",bs,rd_addr);  ++n_errors;  }
		d+=bs;
	}
	return(n_errors);
}


int FX2Reset(uint8_t running)
{
	// Reset is accomplished by writing a 1 to address 0xE600. 
	// Start running by writing a 0 to that address. 
	const size_t reset_addr=0xe600;
	unsigned char val = running ? 0 : 1;
	
	return(WriteRAM(reset_addr,&val,1));
}


int ProgramIHexLine(const char *buf,	const char *path,int line)
{
	const char *s;
	unsigned int nbytes=0,addr=0,type=0;
	unsigned char *data;
	unsigned char cksum;
	unsigned int i,d;
	unsigned int file_cksum;
	
	s=buf;
	if(*s!=':')
	{  fprintf(stderr,"%s:%d: format violation (1)\n",path,line);
		return(1);  }
	++s;
	
	if(sscanf(s,"%02x%04x%02x",&nbytes,&addr,&type)!=3)
	{  fprintf(stderr,"%s:%d: format violation (2)\n",path,line);
		return(1);  }
	s+=8;
	if(type==0)
	{
		//printf("  Writing nbytes=%d at addr=0x%04x\n",nbytes,addr);
		if(!(nbytes>=0 && nbytes<256))
		{  fprintf(stderr,"%s:%d: format violation (2)\n",path,line);
		return(1);  }
		data=malloc(nbytes);
		cksum=nbytes+addr+(addr>>8)+type;
		for(i=0; i<nbytes; i++)
		{
			d=0;
			if(sscanf(s,"%02x",&d)!=1)
			{  fprintf(stderr,"%s:%d: format violation (3)\n",path,line);
				return(1);  }
			s+=2;
			data[i]=d;
			cksum+=d;
		}
		file_cksum=0;
		if(sscanf(s,"%02x",&file_cksum)!=1)
		{  fprintf(stderr,"%s:%d: format violation (4)\n",path,line);
			return(1);  }
		if((cksum+file_cksum)&0xff)
		{  fprintf(stderr,"%s:%d: checksum mismatch (%u/%u)\n",
			path,line,cksum,file_cksum);  return(1);  }
		if(WriteRAM(addr,data,nbytes))
		{ free(data);  return(1);  }
		free(data);
	}
	else if(type==1)
	{
		// EOF marker. Oh well, trust it. 
		return(-1);
	}
	else
	{
		fprintf(stderr,"%s:%d: Unknown entry type %d\n",path,line,type);
		return(1);
	}

	return(0);
}



int usb_init(void)
{
	int found=0;
	int err,line;
	uint8_t b;
	const char *s;
	char **ss;
	if (libusb_init(NULL))
	{
		fprintf(stderr,"libusb Init error\n");
		return -2;
	}
	
	device_h=libusb_open_device_with_vid_pid(NULL,0x04b4,0x8613);
	if(device_h==NULL)
	{
	    fprintf(stderr,"No device found or device already configured.\n");
	}
	else 
	{
	    found=1;
	    err=libusb_set_configuration(device_h,1);
	    if(err)
	    {
		fprintf(stderr,"Can't set device configuration\n");
		return err;
	    }

	    err=libusb_claim_interface(device_h,0);
	    if(err)
	    {
		fprintf(stderr,"Can't claim device interface\n");
		return err;
	    }

	    device=libusb_get_device(device_h);
	    err=libusb_get_active_config_descriptor(device,&conf);
	    if(err)
	    {
		fprintf(stderr,"Can't get device configuration descriptor\n");
		return err;
	    }
	    
	    FX2Reset(0);
	    ss=(void *)vcapt_firmware;
	    err=0;
	    line=0;
	    do
	    {
		s=*ss++;
		if(s!=NULL)
		{
			//printf("%s\n",s);
			err=ProgramIHexLine(s,"vcapt_firmvare",line);
			line++;
		}
	    }while(s);
	    FX2Reset(1);
	    libusb_free_config_descriptor 	(conf);
	    err=libusb_release_interface(device_h,0);
		if(err)
		{
			fprintf(stderr,"Can't release device interface err=%d\n",err);
		}
		libusb_close(device_h);
		libusb_exit(NULL);
		sleep(3);

	}
	if (libusb_init(NULL))
	{
		fprintf(stderr,"libusb Init error\n");
		return -2;
	}
	device_h=libusb_open_device_with_vid_pid(NULL,0xffff,0x2048);
	if(device_h==NULL)
	{
	    fprintf(stderr,"No reconfigured device found\n");
	    return 1;
	}
	err=libusb_set_configuration(device_h,1);
	  if(err)
	    {
		fprintf(stderr,"Can't set device configuration\n");
		return err;
	    }

	err=libusb_claim_interface(device_h,0);
	   if(err)
	    {
		fprintf(stderr,"Can't claim device interface\n");
		return err;
	    }

	device=libusb_get_device(device_h);
	    err=libusb_get_active_config_descriptor(device,&conf);
	    if(err)
	    {
		fprintf(stderr,"Can't get device configuration descriptor\n");
		return err;
	    }
	
	ep2in=conf->interface[0].altsetting[0].endpoint[0].bEndpointAddress;

	
	
	return 0;
}

void usb_done(void)
{
	int err;
	
	libusb_free_config_descriptor 	(conf);
	
	err=libusb_release_interface(device_h,0);
	if(err)
	{
		fprintf(stderr,"Can't release device interface err=%d\n",err);
	}
	libusb_close(device_h);
	libusb_exit(NULL);
	
}




void usb_send_start_cmd(void)
{
	int rv;
	struct cmd_start_acquisition cmd=
	{
		.flags=CMD_START_FLAGS_SAMPLE_8BIT | CMD_START_FLAGS_INV_CLK,
		.sample_delay_h=0,
		.sample_delay_l=0
	};
	rv=libusb_control_transfer(device_h,0x40,CMD_START,
			/*addr=*/0,0,
			/*buf=*/(char*)&cmd,/*size=*/3,
			/*timeout=*/1000/*msec*/);
if(rv<0)
	fprintf(stderr,"Vendor request error\n");
else
	printf("Starting sample data\n");
	
}

#define N 65536
void usb_test(void)
{
	int rv,i;
	struct version_info ver;
	uint8_t *buf;
	int transfered=0;
	FILE *f;
	
	rv=libusb_control_transfer(device_h,0xc0,CMD_GET_FW_VERSION,
			/*addr=*/0,0,
			/*buf=*/(char*)&ver,/*size=*/2,
			/*timeout=*/1000/*msec*/);
if(rv<0)
	{
	    fprintf(stderr,"Vendor request error\n");
	    return;
	}
	printf("Version %d.%d\n",ver.major,ver.minor);
	buf=malloc(N);
	usb_send_start_cmd();

	rv=libusb_bulk_transfer(device_h,ep2in,(char *)buf,N,&transfered,1000);
	if(rv<0)
	{
	    fprintf(stderr,"Bulk transfer error\n");
	    return;
	}
	printf("transfered %lu bytes\n", transfered);
	f=fopen("/tmp/usb_log.dat","w");
	for(i=0;i<transfered;i++)
	{
	    fprintf(f,"%d\n",buf[i]);
	}
	fclose(f);

}


int usb_start_transfer (void) 
{
    uint8_t i;
    uint8_t *usb_buf;
    struct libusb_transfer *xfr;
    
    usb_send_start_cmd();
    for(i=0;i<N_OF_TRANSFERS;i++)
    {
	usb_buf=malloc(USB_BUF_SIZE);
	xfr = libusb_alloc_transfer(0);
	libusb_fill_bulk_transfer(xfr, device_h, ep2in, usb_buf, USB_BUF_SIZE, callbackUSBTransferComplete, NULL, 1000 );
    
	if(libusb_submit_transfer(xfr) < 0)
	{
	    // Error
	    libusb_free_transfer(xfr);
	    free(usb_buf);
	    fprintf(stderr,"USB submit transfer %d error\n",i);
	}
    }
    
    
}

void callbackUSBTransferComplete(struct libusb_transfer *xfr)
{
    switch(xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
            // Success here, data transfered are inside 
            // xfr->buffer
            // and the length is
            // xfr->actual_length
	    data_ready_cb((void *)xfr->buffer, xfr->actual_length);
	    if(libusb_submit_transfer(xfr) < 0)
	    {
		// Error
		libusb_free_transfer(xfr);
		free(xfr->buffer);
		fprintf(stderr,"USB resubmit transfer error\n");
		stop=1;
	    }
            break;
        case LIBUSB_TRANSFER_CANCELLED:
	    fprintf(stderr,"USB transfer error: canceled\n");
	    stop=1;
            break;
        case LIBUSB_TRANSFER_NO_DEVICE:
	    fprintf(stderr,"USB transfer error: no device\n");
	    stop=1;
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
	    fprintf(stderr,"USB transfer error: time out\n");
	    stop=1;
            break;
        case LIBUSB_TRANSFER_ERROR:
	    fprintf(stderr,"USB transfer error\n");
	    stop=1;
            break;
        case LIBUSB_TRANSFER_STALL:
	    fprintf(stderr,"USB transfer error: stall\n");
	    stop=1;
            break;
        case LIBUSB_TRANSFER_OVERFLOW:
            // Various type of errors here
	    fprintf(stderr,"USB transfer error: overflow\n");
	    stop=1;
            break;
    }
}

void usb_poll(viod)  					// need to be periodicaly called
{
    if(libusb_handle_events(NULL) != LIBUSB_SUCCESS)
    {
	fprintf(stderr,"USB error event\n");
    }
}
