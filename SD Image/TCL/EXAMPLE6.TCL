cls;

for {set c 0} {< $c 256} {incr c} {
	fill_rect 0 0 128 128 $c;
	print 0 0 $c;
	delay 100;
}

image 0 0 128 128 "TCL/BENDER4.RAW";

line 128 0 0 128 252;
line 0 0 128 128 224;
line 0 62 128 62 3;

circle 96 32 8 224;

triangle 12 12 48 48 0 50 255;

rect 80 80 40 20 150;

fill_rect 10 80 10 40 100; 

delay 5000;
