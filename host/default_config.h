const char *config_txt={"\
\n\
framebuffer_size=1024;			#framebuffer size is 1024x1024\n\
machines:\n\
(\n\
	{\n\
		name=\"MS-0511\";			\n\
\n\
		clk_inverted=true;			\n\
		inv_bits=0x0f;			# will be xored with input data\n\
		sync_bit_mask=0x10;     # which bit is sync\n\
		pixel_bits_mask=0x0f;	# which bits encode pexel color\n\
		#							 RRGGBB\n\
		colors=({ mask=0x00; color=0x000000; bw=0x00},\n\
				{ mask=0x04; color=0x0000B0; bw=0x2c},\n\
				{ mask=0x01; color=0xB00000; bw=0x42},\n\
				{ mask=0x05; color=0xB000B0; bw=0x58},\n\
				{ mask=0x02; color=0x00B000; bw=0x6e},\n\
				{ mask=0x06; color=0x00B0B0; bw=0x84},\n\
				{ mask=0x03; color=0xB0B000; bw=0x9a},\n\
				{ mask=0x07; color=0xB0B0B0; bw=0xb0},\n\
				{ mask=0x08; color=0x000000; bw=0x00},\n\
				{ mask=0x0C; color=0x0000FF; bw=0x50},\n\
				{ mask=0x09; color=0xFF0000; bw=0x71},\n\
				{ mask=0x0D; color=0xFF00FF; bw=0x8f},\n\
				{ mask=0x0A; color=0x00FF00; bw=0x9f},\n\
				{ mask=0x0E; color=0x00FFFF; bw=0xbf},\n\
				{ mask=0x0B; color=0xFFFF00; bw=0xe3},\n\
				{ mask=0x0F; color=0xFFFFFF; bw=0xff}\n\
				);\n\
		area={ x0=0.06; y0=0.01; x1=0.7; y1=0.3; }    # define rectangle area of raster that will be shown. Full framebuffer size is 1.0 x 1.0\n\
		\n\
	},\n\
	{\n\
		name=\"BK0010-01\";			\n\
\n\
		clk_inverted=true;			\n\
		inv_bits=0x08;			# will be xored with input data\n\
		sync_bit_mask=0x10;     # which bit is sync\n\
		pixel_bits_mask=0x0f;	# which bits encode pexel color\n\
		#							 RRGGBB\n\
		colors=({ mask=0x00; color=0x000000; bw=0x00},\n\
				{ mask=0x04; color=0x0000FF; bw=0x00},\n\
				{ mask=0x01; color=0xFF0000; bw=0x00},\n\
				{ mask=0x05; color=0xFF00FF; bw=0x00},\n\
				{ mask=0x02; color=0x00FF00; bw=0x00},\n\
				{ mask=0x06; color=0x00FFFF; bw=0x00},\n\
				{ mask=0x03; color=0xFFFF00; bw=0x00},\n\
				{ mask=0x07; color=0xFFFFFF; bw=0x00},\n\
				{ mask=0x08; color=0x000000; bw=0xFF},\n\
				{ mask=0x0C; color=0x0000FF; bw=0xFF},\n\
				{ mask=0x09; color=0xFF0000; bw=0xFF},\n\
				{ mask=0x0D; color=0xFF00FF; bw=0xFF},\n\
				{ mask=0x0A; color=0x00FF00; bw=0xFF},\n\
				{ mask=0x0E; color=0x00FFFF; bw=0xFF},\n\
				{ mask=0x0B; color=0xFFFF00; bw=0xFF},\n\
				{ mask=0x0F; color=0xFFFFFF; bw=0xFF}\n\
				);\n\
		area={ x0=0.115; y0=0.035; x1=0.63; y1=0.29; }    # define rectangle area of raster that will be shown. Full framebuffer size is 1.0 x 1.0\n\
		\n\
	},\n\
	{\n\
		name=\"ZX-Spectrum\";			\n\
\n\
		clk_inverted=true;			\n\
		inv_bits=0x00;			# will be xored with input data\n\
		sync_bit_mask=0x10;     # which bit is sync\n\
		pixel_bits_mask=0x0f;	# which bits encode pexel color\n\
		#							 RRGGBB\n\
		colors=({ mask=0x00; color=0x000000; bw=0x00},\n\
				{ mask=0x04; color=0x0000B0; bw=0x2c},\n\
				{ mask=0x01; color=0xB00000; bw=0x42},\n\
				{ mask=0x05; color=0xB000B0; bw=0x58},\n\
				{ mask=0x02; color=0x00B000; bw=0x6e},\n\
				{ mask=0x06; color=0x00B0B0; bw=0x84},\n\
				{ mask=0x03; color=0xB0B000; bw=0x9a},\n\
				{ mask=0x07; color=0xB0B0B0; bw=0xb0},\n\
				{ mask=0x08; color=0x000000; bw=0x00},\n\
				{ mask=0x0C; color=0x0000FF; bw=0x50},\n\
				{ mask=0x09; color=0xFF0000; bw=0x71},\n\
				{ mask=0x0D; color=0xFF00FF; bw=0x8f},\n\
				{ mask=0x0A; color=0x00FF00; bw=0x9f},\n\
				{ mask=0x0E; color=0x00FFFF; bw=0xbf},\n\
				{ mask=0x0B; color=0xFFFF00; bw=0xe3},\n\
				{ mask=0x0F; color=0xFFFFFF; bw=0xff}\n\
				);\n\
		area={ x0=0.048; y0=0.035; x1=0.34; y1=0.245; }    # define rectangle area of raster that will be shown. Full framebuffer size is 1.0 x 1.0\n\
		\n\
	}\n\
)\n\
\n\
\n\
	"};
