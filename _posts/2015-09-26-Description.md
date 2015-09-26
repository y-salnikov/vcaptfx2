---
layout: post
title: "Описание устройства"
comments: false
---

#Аппаратная часть
Устройство собрано на плате LCSOFT CY7C68013A mini board:

![miniboard]({{site.baseurl}}/imgs/miniboard.jpg)

Схема платы:

![Схема]({{site.baseurl}}/imgs/schematics.png)

Из компьютера выводятся цифровые сигналы, содержащие информацию о цвете
пикселей, совмещенный синхросигнал, а также сигнал тактирования пикселей, т.н.
pixel clock.  
Подключение сигналов к плате/микросхеме:

| Вывод микросхемы | Сигнал с компьютера | описание |
|:----------------:|:-------------------:|----------|
| PB0		   | R<sup>*</sup>	| Красная составляющая цвета |
| PB1		   | G<sup>*</sup>	| Зеленая составляющая цвета |
| PB2		   | B<sup>*</sup>	| Синяя составляющая цвета |
| PB3		   | Y<sup>*</sup>	| Сигнал яркости |
| PB4		   | S<sup>*</sup>	| Совмещенный строчный и кадровый синхросигнал |
| IFCLK		   | PXCLK	| Тактовый сигнал, не более 48МГц. |
| RDY0,RDY1	   | +5в		| Сигнал работы компьютера |
| GND		   | Общий		|				|

* - Все сигналы, подключаемые к порту PB0, могут быть подключены к любым выводам порта,
а также могут быть инвертированы. Также нет ограничения на кол-во битов на цвет.
Назначение каждого бита, и его "инвертированность" задается в файле конфигурации, в таблице
приведено мое подключение.


#Программное обеспечение.

Программа состоит из двух частей: прошивка для CY7C68013A, основанная на прошивке
для логического анализатора из проекта sigrok,  и программа показывающая изображение
с компьютера. Изначально разрабатывалась под Linux, однако сейчас есть windows версия.

Программа  использует SDL и OpenGL для масштабируемого отображения видеосигнала,
параметры отображения задаются в конфигурационном файле. Режим отображения цвета переключается
клавишей SCROLL-LOCK. 

Программа поддерживает профили компьютеров, и позволяет хранить параметры отображения
для нескольких компьютеров. При запуске без параметров, выбирается первый профиль. 


Пример файла конфигурации:

```

#common parameters for all machine profiles
common:
{
	vid=0x04b4;						#vid/pid
	pid=0x8613;
	framebuffer_size=1024;			#framebuffer size is 1024x1024
	clk_inverted=true;
	sync_bit_mask=0x10;		# which bit is sync
	inv_bits=0x00;			# will be xored with input data
	pixel_bits_mask=0x0f;	# which bits encode pexel color
# mask in colors can be binary string like "0101" or integer like 0x05 or 5
	colors=({ mask="0000"; color=0x000000; bw=0x00},
			{ mask="0100"; color=0x0000B0; bw=0x2c},
			{ mask="0001"; color=0xB00000; bw=0x42},
			{ mask="0101"; color=0xB000B0; bw=0x58},
			{ mask="0010"; color=0x00B000; bw=0x6e},
			{ mask="0110"; color=0x00B0B0; bw=0x84},
			{ mask="0011"; color=0xB0B000; bw=0x9a},
			{ mask="0111"; color=0xB0B0B0; bw=0xb0},
			{ mask="1000"; color=0x000000; bw=0x00},
			{ mask="1100"; color=0x0000FF; bw=0x50},
			{ mask="1001"; color=0xFF0000; bw=0x71},
			{ mask="1101"; color=0xFF00FF; bw=0x8f},
			{ mask="1010"; color=0x00FF00; bw=0x9f},
			{ mask="1110"; color=0x00FFFF; bw=0xbf},
			{ mask="1011"; color=0xFFFF00; bw=0xe3},
			{ mask="1111"; color=0xFFFFFF; bw=0xff}
			);
	area={ x0=0.00; y0=0.00; x1=1.0; y1=1.0; }    # define rectangle area of raster that will be shown. Full framebuffer size is 1.0 x 1.0
}

# individual machine settings override common

machines:
(
	{
		name="MS-0511";
		inv_bits=0x0f;
		area={ x0=0.07; y0=0.01; x1=0.7; y1=0.3; }
	},
	{
		name="BK0010-01";			

		inv_bits=0x08;			# will be xored with input data
		colors=({ mask=0x00; color=0x000000; bw=0x00},
				{ mask=0x04; color=0x0000FF; bw=0x00},
				{ mask=0x01; color=0xFF0000; bw=0x00},
				{ mask=0x05; color=0xFF00FF; bw=0x00},
				{ mask=0x02; color=0x00FF00; bw=0x00},
				{ mask=0x06; color=0x00FFFF; bw=0x00},
				{ mask=0x03; color=0xFFFF00; bw=0x00},
				{ mask=0x07; color=0xFFFFFF; bw=0x00},
				{ mask=0x08; color=0x000000; bw=0xFF},
				{ mask=0x0C; color=0x0000FF; bw=0xFF},
				{ mask=0x09; color=0xFF0000; bw=0xFF},
				{ mask=0x0D; color=0xFF00FF; bw=0xFF},
				{ mask=0x0A; color=0x00FF00; bw=0xFF},
				{ mask=0x0E; color=0x00FFFF; bw=0xFF},
				{ mask=0x0B; color=0xFFFF00; bw=0xFF},
				{ mask=0x0F; color=0xFFFFFF; bw=0xFF}
				);
		area={ x0=0.115; y0=0.035; x1=0.63; y1=0.29; }
	},
	{
		name="ZX-Spectrum";			

		clk_inverted=true;			
		inv_bits=0x00;			# will be xored with input data
		area={ x0=0.048; y0=0.035; x1=0.34; y1=0.245; }
		
	}
)

```

#Скриншоты


![screenshot]({{site.baseurl}}/screenshots/video capture_009.png)
![screenshot]({{site.baseurl}}/screenshots/video capture_010.png)
![screenshot]({{site.baseurl}}/screenshots/video capture_011.png)
![screenshot]({{site.baseurl}}/screenshots/video capture_013.png)
![screenshot]({{site.baseurl}}/screenshots/video capture_014.png)
![screenshot]({{site.baseurl}}/screenshots/video capture_015.png)
