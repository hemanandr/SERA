import picamera

camera = picamera.PiCamera()

camera.hflip = True
camera.vflip = True

#camera.resolution = (2592, 1944)
camera.resolution = (1024,768)
camera.awb_mode = 'incandescent'
camera.capture('/home/pi/image.jpg')
