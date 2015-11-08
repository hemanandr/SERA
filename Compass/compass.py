import smbus
import math
import os
import time

path = "/home/pi/SERA/Pipes/p_compass";

bus = smbus.SMBus(1);
address = 0x1e;
PI = 3.141592654;

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

	heading = math.atan2(y,x);
	
	if (heading < 0) : heading += 2 * PI;	
	if (heading > 2*PI) : heading -= 2*PI;
		
	angle = heading * 180/PI;

	os.write(pipe,str(angle));
	os.close(pipe);	
	time.sleep(0.05);
