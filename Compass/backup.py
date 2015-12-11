import smbus
import math
import os
import time

path = "/home/pi/SERA/Pipes/p_compass";
p_drive = "/home/pi/SERA/Pipes/p_serial";

bus = smbus.SMBus(1);
address = 0x1e;
PI = 3.141592654;
rad2degree = 57.3;

x_gainError = 0.90;
y_gainError = 0.91;
z_gainError = 0.91;

x_offset = -116.76;
y_offset = -18.36;
z_offset = -18.36;

gain_fact = 1.22;

XY_excitation = 1160;
Z_excitation = 1080;

bus.write_byte_data(address, 0x02, 0x00);
x = 0;
y = 0;
z = 0;
x_scaled = 0;
y_scaled = 0;
z_scaled = 0;

def sint(input):
  if(input > 32768):
    return (input-65536); 
  else:
    return input;

def millis():
  return int(round(time.time() * 1000));

def ReadData():
  global x,y,z;
  data = bus.read_i2c_block_data(address, 0x03);
  x = sint((data[0] << 8) | data[1]);
  z = sint((data[2] << 8) | data[3]);
  y = sint((data[4] << 8) | data[5]);

def calibration():
  #drive = os.open(p_drive, os.O_WRONLY);

  global x_offset, y_offset, z_offset, x_gainError, y_gainError, z_gainError;

  print("Gain Calibration");
    bus.write_byte_data(address,0x00,0b01110001);
  ReadData();

    while(x<200 | y<200 | z<200):
      ReadData();
  
  x_scaled = x * gain_fact;
  y_scaled = y * gain_fact;
  z_scaled = z * gain_fact;
  
  x_gainError = (float) (XY_excitation / x_scaled);
  y_gainError = (float) (XY_excitation / y_scaled);
  z_gainError = (float) (Z_excitation / z_scaled);

  bus.write_byte_data(address,0x00,0b01110010);
  
  ReadData();

    while(x>-200 | y>-200 | z>-200):
      ReadData();
  
  x_scaled = x * gain_fact;
  y_scaled = y * gain_fact;
  z_scaled = z * gain_fact;

    x_gainError = (float)((XY_excitation/abs(x_scaled))+x_gainError)/2;
    y_gainError = (float)((XY_excitation/abs(y_scaled))+y_gainError)/2;
    z_gainError = (float)((Z_excitation/abs(z_scaled))+z_gainError)/2;
  

    print("x_gain_offset = ");
    print(x_gainError);
  print("y_gain_offset = ");
    print(y_gainError);
    print("z_gain_offset = ");
    print(z_gainError);
  
    bus.write_byte_data(address,0x00,0b01111000);
  
    print("Offset Calibration");
  print("Spin");
  #os.write(drive, "d");


    for i in range(0, 10):
      ReadData();

    x_max=(float)(-4000);
    y_max=(float)(-4000);
    z_max=(float)(-4000);
    x_min=(float)(+4000);
    y_min=(float)(+4000);
    z_min=(float)(+4000);

    t = millis();

    while(millis()-t <= 30000):
      ReadData();
        x_scaled= (float)(x*gain_fact*x_gainError);
        y_scaled= (float)(y*gain_fact*y_gainError);
        z_scaled= (float)(z*gain_fact*z_gainError);
        x_max = max(x_max,x_scaled);
        y_max = max(y_max,y_scaled);
        z_max = max(z_max,z_scaled);
        x_min = min(x_min,x_scaled);
        y_min = min(y_min,y_scaled);
        z_min = min(z_min,z_scaled);

  x_offset = (float) (((x_max-x_min)/2)-x_max);
  y_offset = (float) (((y_max-y_min)/2)-y_max);
  z_offset = (float) (((z_max-z_min)/2)-z_max);

  #os.write(drive, "s");

  print("Offset x = ");
  print(x_offset);
  print("Offset y = ");
  print(y_offset);
  print("Offset z = ");
  print(z_offset);


calibration();

while True:
  pipe = os.open(path, os.O_WRONLY);
  
  x_scaled = x * gain_fact * x_gainError + x_offset;
    y_scaled = y * gain_fact * y_gainError + y_offset;

  if (y_scaled > 0):
    angle = 90-math.atan(x_scaled/y_scaled) * rad2degree;
    elif (y_scaled < 0):
      angle = 270-math.atan(x_scaled/y_scaled) * rad2degree;
    elif ((y_scaled==0) & (x_scaled<0)):
      angle = 180;
    else:
      angle = 0;
  
  os.write(pipe,str(angle));
  os.close(pipe); 
  time.sleep(0.05);
