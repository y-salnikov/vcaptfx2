const char* config_txt =
{"\
\n\
#common parameters for all machine profiles\n\
common:\n\
{\n\
    vid=0x04B4; #vid/pid\n\
    pid=0x8613;\n\
    fullscreen_width=1920;\n\
    fullscreen_height=1080;\n\
}\n\
\n\
# individual machine settings override common\n\
\n\
machines:\n\
(\n\
    {\n\
        name=\"MS-0511\";\n\
        frame_width=640;\n\
        frame_height=288;\n\
        h_counter_shift=-73;\n\
        v_counter_shift=-17;\n\
        clk_inverted=true;\n\
        sync_bit_mask=0x80;   # which bit is sync\n\
        inv_bits=0x47;        # will be xored with input data\n\
        pixel_bits_mask=0x7F; # which bits encode pixel color\n\
    }\n\
)\n\
"
};
