cls;

set base 42;
set w 4;
set x 0;
set y 60;
print [* $w $x] $y [chr [+ $base 79]];
incr x;
print [* $w $x] $y [chr [+ $base 53]];
incr x;
print [* $w $x] $y [chr [+ $base 23]];
incr x;
print [* $w $x] $y [chr [+ $base 36]];
incr x;
print [* $w $x] $y [chr [- 100 $base]];
incr x;
print [* $w $x] $y [chr [+ 66 $base]];
incr x;
print [* $w $x] $y [chr [+ 72 $base]];
incr x;
print [* $w $x] $y [chr [+ 36 $base]];
incr x;

delay 5000;


