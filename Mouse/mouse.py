import struct

file = open( "/dev/input/mice", "rb" );

point_x = 0;
point_y = 0;

class Point:
	x = 0.0
	y = 0.0

def event():
  	buf = file.read(4);
  	button = ord( buf[3] );
  	print("%d" % button);

def getMouseEvent():
  	buf = file.read(3);
  	x,y = struct.unpack( "bb", buf[1:] );
  	dis = Point();
  	dis.x = x;
  	dis.y = y;
  	return dis;

while( 1 ):
  	event();
  	#dis = getMouseEvent();
	#point_x = point_x + (0.0254 * dis.x);
	#point_y = point_y + (0.0254 * dis.y);
  	#print ("%d   %d\n" % (distance.x, distance.y));

file.close();