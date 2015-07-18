#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libconfig.h>
#include "types.h"
#include "machine.h"
#include "compat.h"
#include "default_config.h"

const uint8_t bw_colors[16]={0x00,0x2c,0x42,0x58,0x6e,0x84,0x9a,0xb0,0x00,0x50,0x71,0x8f,0x9f,0xbf,0xe3,0xff};         // magic brightness constants for my monitor




uint8_t convert_bw(uint8_t r, uint8_t g, uint8_t b, uint8_t y)
{
	uint8_t i,c;
	i=0;
	if(b) i+=1;
	if(r) i+=2;
	if(g) i+=4;
	if(y) i+=8;
	c=bw_colors[i];
	return c;
}

void machine_get_area(machine_type *mac, float *x0, float *y0, float *x1, float *y1)
{
	*x0=mac->x0;
	*y0=mac->y0;
	*x1=mac->x1;
	*y1=mac->y1;
}


void extract_color(machine_type *mac, uint8_t d, uint8_t *R, uint8_t *G, uint8_t *B)
{
	uint8_t data,i;
	data=(d ^ mac->inv_bits) & mac->pixel_bits_mask;
	for(i=0;i<mac->colors_length;i++)
	{
		if(data==mac->colors[i].mask)
		{
			if(mac->color_mode)
			{
				*R=mac->colors[i].R;
				*G=mac->colors[i].G;
				*B=mac->colors[i].B;
				return;
			}else
			{
				*R=*G=*B=mac->colors[i].BW;
				return;
			}
			
		}
	}
	*R=*G=*B=0;
}



FILE *open_cfg_file(const char *PATH, const char *FILENAME)
{
	char *full_path;
	full_path=malloc(65536);
	FILE *f;
	if (PATH)
	{
		#ifdef WIN32
			CreateDirectory(PATH,NULL);
		#elif UNIX
			mkdir(PATH,0755);
			
		#endif
	}
	
	strcpy(full_path,PATH);
	strcat(full_path,FILENAME);
	
	if (is_file_exist(full_path))
	{
		printf("Open existing config file: %s\n",full_path);
		return fopen(full_path,"rb");
	}
	f=fopen(full_path,"wb");
	fprintf(f,"%s",config_txt);
	fclose(f);
	printf("New default config written: %s\n",full_path);
	return fopen(full_path,"rb");
}


machine_type *machine_init(uint8_t command, const char* machine_name, const char* config_file_path)
{
	char *endptr=NULL;
	long int mach_index=0;
	machine_type *mac;
	const char* cfg_path=NULL;
	const char* cfg_filename=NULL;
	const char* mac_name=NULL;
	FILE *config_file;
	config_t cfg; 
	config_setting_t *setting,*machine, *colors, *color, *area;
	int count,i;
	uint8_t found,err;
	int c,m,bw;

	err=0;
	
	mac=malloc(sizeof (machine_type));
	if((command==2) || (command==3))	// -m
	{
		mach_index=strtol(machine_name,&endptr,0);
		if(endptr==machine_name) mach_index=0;
	}
	if(command==3)						// -m -f
	{
		cfg_path="";
		cfg_filename=config_file_path;
	}else
	{
		cfg_path=get_config_file_path();
		cfg_filename="machines.cfg";
	}
	config_file=open_cfg_file(cfg_path,cfg_filename);
	if(config_file==NULL) return NULL;
	config_init(&cfg);
	if(! config_read(&cfg, config_file))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return NULL;
	}
	setting = config_lookup(&cfg, "machines");

	if(setting != NULL)
	{
		count = config_setting_length(setting);
		
	}
	else
	{
		fprintf(stderr,"machines not found in config\n");
		config_destroy(&cfg);
		return NULL;
	}
	
	if (command==1)
	{
		for(i=0;i<count;i++)
		{
			machine=config_setting_get_elem(setting,i);
			if( config_setting_lookup_string(machine, "name",&mac_name)==CONFIG_TRUE)
			{
				printf("%d: %s\n",i+1,mac_name);
			}
			
		}
		return NULL;
	}
	if (mach_index>0)    //select by index
	{
			machine=config_setting_get_elem(setting,mach_index-1);
			if(machine==NULL)
			{
				fprintf(stderr,"Index out of range. Use --list to check.\n");
				config_destroy(&cfg);
				return NULL;
			}
	}else				// by name
	{	found=0;
		for(i=0;i<count;i++)
		{
			machine=config_setting_get_elem(setting,i);
			if( config_setting_lookup_string(machine, "name",&mac_name)==CONFIG_TRUE)
			{
				if(!(strcmp(mac_name,machine_name)))
				{	found++;
					 break;
				}
			}
			
		}
		if(found==0)
		{
					fprintf(stderr,"%s not found\n",machine_name);
					return NULL;
		}
	}

	if(config_setting_lookup_string(machine, "name",&mac_name)!=CONFIG_TRUE)
	{
		fprintf(stderr,"machine name not found in config\n");
		config_destroy(&cfg);
		return NULL;
	}
	printf("%s  selected\n",mac_name);
	mac->name=malloc(256);
	strcpy(mac->name,mac_name);
	if( (config_setting_lookup_bool(machine, "clk_inverted",&mac->clk_inverted )) && (config_setting_lookup_int(machine, "inv_bits",&mac->inv_bits)) && (config_setting_lookup_int(machine, "sync_bit_mask",&mac->sync_bit_mask)) && (config_setting_lookup_int(machine,"pixel_bits_mask",&mac->pixel_bits_mask )))
	{
		colors=config_setting_get_member(machine,"colors");
		if (colors==NULL) err++;
		else
		{
			mac->colors_length=config_setting_length(colors);
			mac->colors=calloc(mac->colors_length,sizeof(mach_colors));
			
			for(i=0;i<mac->colors_length;i++)
			{
				color=config_setting_get_elem(colors,i);
				if (!((config_setting_lookup_int(color,"mask",&m)) && (config_setting_lookup_int(color,"color",&c)) &&
				   (config_setting_lookup_int(color,"bw",&bw))))
				{
					err++;
					break;
				}
				mac->colors[i].mask=( m & 0xFF);
				mac->colors[i].R=((c & 0xFF0000) >>16);
				mac->colors[i].G=((c & 0x00FF00) >>8);
				mac->colors[i].B=((c & 0xFF) );
				mac->colors[i].BW=((bw & 0xFF));
			}
		}
		area=config_setting_get_member(machine,"area");
		if(area==NULL) err++;
		else
		{
			if(!((config_setting_lookup_float(area,"x0",&mac->x0)) && (config_setting_lookup_float(area,"y0",&mac->y0)) && (config_setting_lookup_float(area,"x1",&mac->x1)) && (config_setting_lookup_float(area,"y1",&mac->y1)))) err++;
		}
	}
	else err++;

	
	if(config_lookup_int(&cfg,"framebuffer_size",&mac->fb_size)!=CONFIG_TRUE) mac->fb_size=1024;
	
	if(err)
	{
		fprintf(stderr,"Error in config\n");
		config_destroy(&cfg);
		return NULL;
	}

	
	mac->color_mode=1;
	

	
	return mac;
}

void machine_done(machine_type *mac)
{
	free(mac->name);
	free(mac->colors);
	free(mac);
}
