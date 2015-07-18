# vcaptfx2
This repository contain software for cypress fx2lp based device for capturing video stream from old zx-spectrum-like computers.

# Hardware

Device based on LCSOFT CY7C68013A mini board http://www.lctech-inc.com/Hardware/Detail.aspx?id=7cadef7b-359a-4e3d-b980-76ffa15171e0
but it can be any board with cypress FX2 chip.
In fact it is just synchronous data logger. Colour and luminance signals (up to 7) and combined sync connect to PB0-PB7 pins of FX2, and pixel clock connect to ifclk pin.
I have:  R - PB0,  G - PB1, B - PB2, L - PB3,   S - PB4, clk - ifclk.

# Software
As firmware for FX2 chip used slightly modified fx2lafw version from sigrok project. http://sigrok.org
The ifclk pin now input, eliminated wait states so each clock pulse now sample data. 
Host side software uses libusb to acquire data from FX2 and render image using OpenGL.
It is written for GNU/Linux but should be easy to port.
Default config represets my few computers: МС-0511(УКНЦ), БК0010-01 and КР-05 (zx-spectrum clone).
