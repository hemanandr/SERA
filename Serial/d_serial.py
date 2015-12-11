
import serial
import os
import errno
import time

path = "/home/pi/SERA/Pipes/p_serial";	#path to the pipe used for accessing serial communication
output = "/home/pi/SERA/Pipes/p_output";

ports = ('ACM1', 'ACM0');
index = 0;
trial = 0;

def Connect():
	global index, trial;
	try:
		temp =  serial.Serial(port='/dev/tty' + ports[index], baudrate=9600, timeout=0);	
		trial = 0;
		return temp;
	except serial.serialutil.SerialException as err:
		trial += 1;
		if(trial > (len(ports) - 1)):
			print("Exit");
			exit();
		index += 1;
		if(index > (len(ports) - 1)): index = 0;
		return Connect();

ser = Connect();

while 1:
	try:
		pipe = os.open(path, os.O_RDONLY | os.O_NONBLOCK);
		input = os.read(pipe,10);
	except OSError as err:
		if err.errno == 11:
			continue;
		else:
			raise err;
	
	if input:
		try:	
			ser.write(input);
			time.sleep(0.1);
		except serial.serialutil.SerialException as err:
			ser = Connect();
			ser.write(input);
	os.close(pipe);

	
	arduino = ser.readline();

	if arduino:
		try:
			pipe = os.open(output, os.O_WRONLY | os.O_NONBLOCK);
			os.write(pipe, arduino);
			os.close(pipe);
		except OSError as err:
			if err.errno == 6:
				pass;
			else:
				raise err;

	
