import serial
import os
import errno

path = "/home/pi/SERA/Pipes/pipe";

ser =  serial.Serial(port='/dev/ttyACM1', baudrate=9600, timeout=0);
count = 0;
pipe = os.open(path, os.O_RDONLY | os.O_NONBLOCK);

while 1:
	pipe = os.open(path, os.O_RDONLY | os.O_NONBLOCK);
	try:
		input = os.read(pipe,10);
	except OSError as err:
		if err.errno == 11:
			import time
			time.sleep(0)
		else:
			raise err;

	if input:
		ser.write(input[0]);
		
	os.close(pipe);
