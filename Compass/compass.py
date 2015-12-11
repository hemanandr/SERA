import smbus
import math
import os
import time

path = "/home/pi/SERA/Pipes/p_compass";

bus = smbus.SMBus(1);
address = 0x1e;
PI = 3.141592654;
rad2degree = 57.3;

x_gainError = 1;
y_gainError = 1;

x_offset = 0;
y_offset = 0;

gain_fact = 1.22;

bus.write_byte_data(address, 0x02, 0x00);

def sint(input):
	if(input > 32768):
		return (input-65536); 
	else:
		return input;

while True:
	pipe = os.open(path, os.O_WRONLY);
	data = bus.read_i2c_block_data(address, 0x03);
	x = sint((data[0] << 8) | data[1]);
	y = sint((data[4] << 8) | data[5]);

	x_scaled= x * gain_fact * x_gainError + x_offset;
  	y_scaled= y * gain_fact * y_gainError + y_offset;

	if (y_scaled > 0):
		angle = 90-math.atan(x_scaled/y_scaled) * rad2degree;
  	elif (y_scaled < 0):
  		angle = 270-math.atan(x_scaled/y_scaled) * rad2degree;
  	elif (y_scaled==0 & x_scaled<0):
  		angle = 180;
  	else:
  		angle = 0;
  
	os.write(pipe,str(angle));
	os.close(pipe);	
	time.sleep(0.05);