cls;
print 0, 60 "The Eyes!";

for {set h 0} {< $h 100} {incr h} {
	led_set_hsv 6 $h 100 100; 
	led_set_hsv 12 $h 100 100;
	delay 100;	
};
