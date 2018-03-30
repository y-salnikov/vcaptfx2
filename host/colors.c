#include "types.h"

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

void colors_bw(match_color colors[], int idx)
{
    match_color base_colors[4] = {
        {0xFF, 0xEB, 0x10},
        {0x11, 0xFF, 0x94},
        {0x19, 0xFF, 0x19},
        {0xFF, 0xFF, 0xFF}
    };

    for(int i = 0; i < 128; i++)
    {
        int c = swap_rg(i);
        colors[i].R = (c & 0x07) * (base_colors[idx].R / 7);
        colors[i].G = (c & 0x07) * (base_colors[idx].G / 7);
        colors[i].B = (c & 0x07) * (base_colors[idx].B / 7);
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
    const uint8_t C = 0x58; // 0x80
    const uint8_t L = 0x27; // 0x2B;
    const uint8_t Y = 0x54; // 0x54;
    match_color color;

    for(int i = 0; i < 128; i++)
    {
        color.R = color.G = color.B = 0;

        if (i & 0x07 == 0) {
            colors[i] = color;
            continue;
        }

        if (i & 0x04) { color.R += C; }
        if (i & 0x02) { color.G += C; }
        if (i & 0x01) { color.B += C; }

        if (i & 0x20) { color.R += L; }
        if (i & 0x10) { color.G += L; }
        if (i & 0x08) { color.B += L; }

        if ((i & 0x40) && (i & 0x24)) { color.R *= 2; }
        if ((i & 0x40) && (i & 0x12)) { color.G *= 2; }
        if ((i & 0x40) && (i & 0x09)) { color.B *= 2; }

        if (grb)
            colors[swap_rg(i)] = color;
        else
            colors[i] = color;
    }
}
