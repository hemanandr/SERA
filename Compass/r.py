import smbus
import math
import os
import time

path = "/home/pi/SERA/Pipes/p_compass";

while True:
	pipe = os.open(path, os.O_RDONLY);
	print (os.read(pipe,100));
	os.close(pipe);
#	time.sleep(0.5);
