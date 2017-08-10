cls;
print 0 0 "Toggling IO 0 Low/High 100 times"

for {set i 0} {< $i 100} {incr i} {
	fill_rect 0 64 128 20 0
	print 0 64 $i
	io_write 0 HIGH;
	delay 400;
	io_write 0 LOW;
	delay 400;
}
