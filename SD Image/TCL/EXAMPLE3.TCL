cls;
print 0 0 "Hold any button to quit rick roll";
delay 2000;

while {== [button_state] 0} {
	play "/BLING/AND!XOR/RICKROLL.RAW";
};
