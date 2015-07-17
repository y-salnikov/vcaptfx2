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
#include "command.h"
#include "compat.h"
#include "types.h"
#include "process.h"
#include "usb.h"

void callbackUSBTransferComplete(struct libusb_transfer *xfr);

int WriteRAM(usb_transfer_context_type *utc, size_t addr,const unsigned char *data, size_t nbytes)
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
		rv=libusb_control_transfer(utc->device_h,0x40,0xa0,
			/*addr=*/dl_addr,0,
			/*buf=*/(unsigned char*)d,/*size=*/bs,
			/*timeout=*/1000/*msec*/);
		if(rv<0)
		{  fprintf(stderr,"Error: Writing %lu bytes at 0x%X\n",(long unsigned int)bs,(unsigned int)dl_addr);  ++n_errors;  }
		d+=bs;
	}
	return(n_errors);
}


int ReadRAM(usb_transfer_context_type *utc, size_t addr,const unsigned char *data, size_t nbytes)
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
		rv=libusb_control_transfer(utc->device_h,0xc0,0xa0,
			/*addr=*/rd_addr,0,
			/*buf=*/(unsigned char*)d,/*size=*/bs,
			/*timeout=*/1000/*msec*/);
		if(rv<0)
		{  fprintf(stderr,"Error: Reading %lu bytes at 0x%X\n",(long unsigned int)bs,(unsigned int)rd_addr);  ++n_errors;  }
		d+=bs;
	}
	return(n_errors);
}


int FX2Reset(usb_transfer_context_type *utc,uint8_t running)
{
	// Reset is accomplished by writing a 1 to address 0xE600. 
	// Start running by writing a 0 to that address. 
	const size_t reset_addr=0xe600;
	unsigned char val = running ? 0 : 1;
	
	return(WriteRAM(utc,reset_addr,&val,1));
}


int ProgramIHexLine(usb_transfer_context_type *utc, const char *buf,	const char *path,int line)
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
		if(WriteRAM(utc,addr,data,nbytes))
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



usb_transfer_context_type*  usb_init(const char **firmware, void *proc_cont)
{

	usb_transfer_context_type *utc;
	int err,line;
	const char *s;
	const char **ss;
	
	if (libusb_init(NULL))
	{
		fprintf(stderr,"libusb Init error\n");
		return NULL;
	}
	utc=malloc(sizeof(usb_transfer_context_type));
	utc->active_transfers=0;
	utc->usb_transfer_cb_served=0;
	utc->usb_stop_flag=2;   // 0 -running, 1-stoppes, 2-not inited
	utc->process_context=proc_cont;
	utc->device_h=libusb_open_device_with_vid_pid(NULL,0x04b4,0x8613);
	if(utc->device_h==NULL)
	{
	    fprintf(stderr,"No device found or device already configured.\n");
	}
	else 
	{

	    err=libusb_set_configuration(utc->device_h,1);
	    if(err)
	    {
		fprintf(stderr,"Can't set device configuration\n");
		return NULL;
	    }

	    err=libusb_claim_interface(utc->device_h,0);
	    if(err)
	    {
		fprintf(stderr,"Can't claim device interface\n");
		return NULL;
	    }

	    utc->device=libusb_get_device(utc->device_h);
	    err=libusb_get_active_config_descriptor(utc->device,&utc->conf);
	    if(err)
	    {
		fprintf(stderr,"Can't get device configuration descriptor\n");
		return NULL;
	    }
	    
	    FX2Reset(utc,0);
	    ss=firmware;
	    err=0;
	    line=0;
	    do
	    {
		s=*ss++;
		if(s!=NULL)
		{
			//printf("%s\n",s);
			err=ProgramIHexLine(utc,s,"vcapt_firmvare",line);
			line++;
		}
	    }while(s);
	    FX2Reset(utc,1);
	    libusb_free_config_descriptor 	(utc->conf);
	    err=libusb_release_interface(utc->device_h,0);
		if(err)
		{
			fprintf(stderr,"Can't release device interface err=%d\n",err);
		}
		libusb_close(utc->device_h);
		libusb_exit(NULL);
		 //wait 3 sec for renumeration
		SLEEP(3000);

	}
	if (libusb_init(NULL))
	{
		fprintf(stderr,"libusb Init error\n");
		return NULL;
	}
	utc->device_h=libusb_open_device_with_vid_pid(NULL,0xffff,0x2048);
	if(utc->device_h==NULL)
	{
	    fprintf(stderr,"No reconfigured device found\n");
	    return NULL;
	}
	err=libusb_set_configuration(utc->device_h,1);
	  if(err)
	    {
		fprintf(stderr,"Can't set device configuration\n");
		return NULL;
	    }

	err=libusb_claim_interface(utc->device_h,0);
	   if(err)
	    {
		fprintf(stderr,"Can't claim device interface\n");
		return NULL;
	    }

	utc->device=libusb_get_device(utc->device_h);
	    err=libusb_get_active_config_descriptor(utc->device,&utc->conf);
	    if(err)
	    {
		fprintf(stderr,"Can't get device configuration descriptor\n");
		return NULL;
	    }
	
	utc->endpoint=utc->conf->interface[0].altsetting[0].endpoint[0].bEndpointAddress;

	return utc;
}

void usb_done(usb_transfer_context_type *utc)
{
	int err;
	
	libusb_free_config_descriptor 	(utc->conf);
	
	err=libusb_release_interface(utc->device_h,0);
	if(err)
	{
		fprintf(stderr,"Can't release device interface err=%d\n",err);
	}
	libusb_close(utc->device_h);
	libusb_exit(NULL);
	free(utc);
}




void usb_send_start_cmd(usb_transfer_context_type *utc)
{
	int rv;
	struct cmd_start_acquisition cmd=
	{
		.flags=CMD_START_FLAGS_SAMPLE_8BIT | CMD_START_FLAGS_INV_CLK,
		.sample_delay_h=0,
		.sample_delay_l=0
	};
	rv=libusb_control_transfer(utc->device_h,0x40,CMD_START,
			/*addr=*/0,0,
			/*buf=*/(unsigned char*)&cmd,/*size=*/3,
			/*timeout=*/1000/*msec*/);
if(rv<0)
	fprintf(stderr,"Vendor request error\n");
else
	printf("Starting sample data\n");
	
}

#define N 65536
void usb_test(usb_transfer_context_type *utc)
{
	int rv,i;
	struct version_info ver;
	uint8_t *buf;
	int transfered=0;
	FILE *f;
	
	rv=libusb_control_transfer(utc->device_h,0xc0,CMD_GET_FW_VERSION,
			/*addr=*/0,0,
			/*buf=*/(unsigned char*)&ver,/*size=*/2,
			/*timeout=*/1000/*msec*/);
if(rv<0)
	{
	    fprintf(stderr,"Vendor request error\n");
	    return;
	}
	printf("Version %d.%d\n",ver.major,ver.minor);
	buf=malloc(N);
	usb_send_start_cmd(utc);

	rv=libusb_bulk_transfer(utc->device_h,utc->endpoint,(unsigned char *)buf,N,&transfered,1000);
	if(rv<0)
	{
	    fprintf(stderr,"Bulk transfer error\n");
	    return;
	}
	printf("transfered %d bytes\n", transfered);
	f=fopen("/tmp/usb_log.dat","w");
	for(i=0;i<transfered;i++)
	{
	    fprintf(f,"%d\n",buf[i]);
	}
	fclose(f);

}


void usb_start_transfer (usb_transfer_context_type *utc) 
{
    uint8_t i;
    uint8_t *usb_buf;
    struct libusb_transfer *xfr;
    
    usb_send_start_cmd(utc);
    for(i=0;i<N_OF_TRANSFERS;i++)
    {
		usb_buf=malloc(USB_BUF_SIZE);
		xfr = libusb_alloc_transfer(0);
		libusb_fill_bulk_transfer(xfr, utc->device_h, utc->endpoint, usb_buf, USB_BUF_SIZE, callbackUSBTransferComplete, utc, 1000 );
	    
		if(libusb_submit_transfer(xfr) < 0)
		{
		    // Error
		    libusb_free_transfer(xfr);
		    free(usb_buf);
		    fprintf(stderr,"USB submit transfer %d error\n",i);
		}
		else utc->active_transfers++;
    }
    
    
}

void callbackUSBTransferComplete(struct libusb_transfer *xfr)
{
	uint8_t err=0;
	usb_transfer_context_type *utc;

	utc=xfr->user_data;
    switch(xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
            // Success here, data transfered are inside 
            // xfr->buffer
            // and the length is
            // xfr->actual_length
	    parse_data(utc->process_context,(void *)xfr->buffer, xfr->actual_length);
	    if(libusb_submit_transfer(xfr) < 0)
	    {
			// Error
			libusb_free_transfer(xfr);
			free(xfr->buffer);
			fprintf(stderr,"USB resubmit transfer error\n");
			utc->active_transfers--;
			err=1;
	    }
        break;
        case LIBUSB_TRANSFER_CANCELLED:
			fprintf(stderr,"USB transfer error: canceled\n");
			err=1;
        break;
        case LIBUSB_TRANSFER_NO_DEVICE:
			if(utc->usb_stop_flag!=1) fprintf(stderr,"USB transfer error: no device\n");
			err=1;
        break;
        case LIBUSB_TRANSFER_TIMED_OUT:
			fprintf(stderr,"USB transfer error: time out\n");
	    	err=1;
        break;
        case LIBUSB_TRANSFER_ERROR:
			fprintf(stderr,"USB transfer error\n");
	    	err=1;
        break;
        case LIBUSB_TRANSFER_STALL:
			fprintf(stderr,"USB transfer error: stall\n");
	    	err=1;
        break;
        case LIBUSB_TRANSFER_OVERFLOW:
			fprintf(stderr,"USB transfer error: overflow\n");
	    	err=1;
            break;
    }
	if (err && utc->usb_stop_flag==0)
	{
		fprintf(stderr,"%d active transfers left\n", --utc->active_transfers);
		free(xfr->buffer);
	}
    if (utc->active_transfers<2) utc->usb_stop_flag=1;
    utc->usb_transfer_cb_served=1;
}


void usb_poll(void)  					// need to be periodicaly called
{
    if(libusb_handle_events(NULL) != LIBUSB_SUCCESS)
    {
	fprintf(stderr,"USB error event\n");
    }
}

int usb_thread_function(void *utc_ptr)
{
	usb_transfer_context_type *utc;
	utc=utc_ptr;
	usb_start_transfer(utc);
	utc->usb_stop_flag=0;
	while(!utc->usb_stop_flag)
	{
		utc->usb_transfer_cb_served=1;
		while(utc->usb_transfer_cb_served)	usb_poll();
		SLEEP(1);
	}
	return 0;
}

uint8_t usb_get_thread_state(usb_transfer_context_type *utc)
{
	return utc->usb_stop_flag;
}

void usb_stop_thread(usb_transfer_context_type *utc)
{
	utc->usb_stop_flag=1;
}
