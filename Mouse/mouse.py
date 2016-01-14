import struct
import math
import os
import errno

file = open( "/dev/input/mice", "rb" );
output = "/home/pi/SERA/Pipes/p_mouse";

point_x = 0;
point_y = 0;
tx = 0;
ty = 0;

class Point:
	x = 0.0
	y = 0.0

def getMouseEvent():
  	buf = file.read(3);
  	x,y = struct.unpack( "bb", buf[1:] );
  	dis = Point();
  	dis.x = x;
  	dis.y = y;
  	return dis;

while( 1 ):
  	dis = getMouseEvent();
	point_x = point_x + (0.042 * dis.x);
	point_y = point_y + (0.042 * dis.y);
	tx = tx + dis.x;
	ty = ty + dis.y;
#	print ("%d  %d" % (tx, ty));
 	print ("%dcm  %dcm\n" % (point_x/10, point_y/10));

	try:
		pipe = os.open(output, os.O_WRONLY | os.O_NONBLOCK);
		os.write(pipe, "%d %d" % (point_x/10,point_y/10));
		os.close(pipe);
	except OSError as err:
		if err.errno == 6:
			pass;
		else:
			raise err;

file.close();
