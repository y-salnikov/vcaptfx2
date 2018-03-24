# Скрипт генерирующий мапинг всех 128-ми цветов УКНЦ

header = <<-STR
#common parameters for all machine profiles
common:
{
    vid=0x04b4; #vid/pid
    pid=0x8613;
    usb_freq=12000000;
    frame_width=640;
    frame_height=288;
    h_counter_shift=-73;
    v_counter_shift=-17;
    fullscreen_width=1400;
    fullscreen_height=1050;
    clk_inverted=true;
    sync_bit_mask=0x80;   # which bit is sync
    inv_bits=0x00;        # will be xored with input data
    pixel_bits_mask=0x7f; # which bits encode pixel color
# mask in colors can be binary string like "0101" or integer like 0x05 or 5
# SS Y RL GL BL R G B
        colors=(
STR

footer = <<-STR
    );
}

# individual machine settings override common

machines:
(
  {
    name="MS-0511";
    inv_bits=0x47;
    usb_freq=12500000;
  }
)
STR

f = File.open(ENV['HOME'] + '/.config/vcaptfx2/machines.cfg', 'w')
f.puts header
c = 128 & 0xFF #160
l =  43 & 0xFF # 31
y =  84 & 0xFF # 64

# SS Y RL GL BL R G B
0b0000000.upto(0b1111111) do |mask|
  f.puts ',' if mask > 0

  color  = 0x000000
  bw = 0x00
  bw += 0x2F if (mask & 0b0000001) > 0
  bw += 0x4F if (mask & 0b0000010) > 0
  bw += 0x7F if (mask & 0b0000100) > 0

  color += c <<  0 if (mask & 0b0000001) > 0
  color += c <<  8 if (mask & 0b0000010) > 0
  color += c << 16 if (mask & 0b0000100) > 0

  color += l <<  0 if (mask & 0b0001000) > 0 && (mask & 0b0000001) > 0
  color += l <<  8 if (mask & 0b0010000) > 0 && (mask & 0b0000010) > 0
  color += l << 16 if (mask & 0b0100000) > 0 && (mask & 0b0000100) > 0

  color += y <<  0 if (mask & 0b1000000) > 0 && (mask & 0b0000001) > 0
  color += y <<  8 if (mask & 0b1000000) > 0 && (mask & 0b0000010) > 0
  color += y << 16 if (mask & 0b1000000) > 0 && (mask & 0b0000100) > 0

  str = ' ' * 17
  str += %({ mask="#{mask.to_s(2).rjust(7, '0')}";)
  str += %( color=0x#{color.to_s(16).rjust(6, '0').upcase};)
  str += %( bw=0x#{bw.to_s(16).rjust(2, '0').upcase} })
  f.print str
end

f.puts
f.puts footer

f.close
