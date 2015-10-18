#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <math.h>  
#include "pixy.h" 
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <ctime>

using namespace cv;
using namespace std;

#define FOCAL_LENGTH    0.28
#define FRAME_HEIGHT    200
#define SENSOR_HEIGHT   0.243
#define ANGLE_PER_PIXEL_X 75.0/320.0
#define ANGLE_PER_PIXEL_Y 47.0/200.0

#define PI              3.14159265

#define OBJECT_HEIGHT   15.0
#define OBJECT_WIDTH    3.0

#define FIELD_DIMENSION 300

void interpolateBayer(unsigned int width, unsigned int x, unsigned int y, unsigned char *pixel, unsigned int &r, unsigned int &g, unsigned int &b);
Mat renderBA81(uint8_t renderFlags, uint16_t width, uint16_t height, uint32_t frameLen, uint8_t *frame);
void mouseEvent(int evt, int x, int y, int flags, void* param) ;
Mat getImage();
Rect getObject();
Point location(float realHeight, Rect object);
void printMap(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION]);
void mapBot(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION]);
void mapObject(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION], int8_t x, int8_t y);

uint8_t botx = 150, boty = 150;

int main(int argc, char * argv[])
{
    int pixy_init_status;
    int return_value;
    int32_t response;

    uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION]; //map[y][x]
    

    for(int y=0; y<FIELD_DIMENSION; y++)
    {
        for(int x=0; x<FIELD_DIMENSION; x++)
        {
            map[y][x] = 0;
        }
    }

    mapBot(map);

    pixy_init_status = pixy_init();
  
    if(!pixy_init_status == 0)
    {
        printf("pixy_init(): ");
        pixy_error(pixy_init_status);
        return pixy_init_status;
    }

    return_value = pixy_command("stop", END_OUT_ARGS, &response, END_IN_ARGS); 
    return_value = pixy_rcs_set_position(1, 900);
    return_value = pixy_rcs_set_position(0, 500);
    //return_value = pixy_led_set_RGB(255,255,255);
    //return_value = pixy_led_set_max_current(20000);

    Rect object;
    
    /*while( object.y < 10)
    {
        return_value = pixy_rcs_set_position(1, pixy_rcs_get_position(1) - 50);
        object = getObject();
    }

    while( object.y + object.height > 190)
    {
        return_value = pixy_rcs_set_position(1, pixy_rcs_get_position(1) + 50);
        object = getObject();
    }*/

    
    
    while(1)
    {
        Point objectLocation;
        time_t now = time(0);
        char* dt = ctime(&now);
        
        object = getObject();
        
        //If Height / Width ratio not within the error range of 0.5 ignore object
        //if( ( (float)object.height/(float)object.width > (OBJECT_HEIGHT/OBJECT_WIDTH)+0.5) || ( (float)object.height/(float)object.width < (OBJECT_HEIGHT/OBJECT_WIDTH)-0.5))
            //continue; 

        objectLocation = location(OBJECT_HEIGHT, object);
        
        printf("%s -> X : %i ; Y : %i\n", dt, objectLocation.x, objectLocation.y);
        
        #if 0
        printf("X:%i Y:%i Height:%i Width:%i\n", object.x, object.y, object.height, object.width); 
        printf("%f %g\n", (float)object.height/(float)object.width, OBJECT_HEIGHT/OBJECT_WIDTH);
        #endif

        mapObject(map, objectLocation.x, objectLocation.y);
    }
    

    printMap(map);
    pixy_close();
    exit(0);
  
}

void mapBot(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION])
{
    uint8_t length = 40;
    uint8_t width = 30;

    for(int i = boty - length/2; i < boty + length/2; i++)
    {
        for(int j = botx - width/2; j < botx + width/2; j++)
        {
            map[i][j] = 1;
        }
    }
}

void mapObject(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION], int8_t x, int8_t y)
{
    uint8_t length = 40;
    uint8_t width = 4;

    for(int i = y + boty + length/2; i < y + width + boty + length/2; i++)
    {
        for(int j = x + botx - width/2; j < x + botx + width/2; j++)
        {
            map[i][j] = 2;
        }
    }
}

void printMap(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION])
{
    uchar data[FIELD_DIMENSION*FIELD_DIMENSION*3];
    Mat image;

    uint m=0;

    for(int y=0; y<FIELD_DIMENSION; y++)
    {
        for(int x=0; x<FIELD_DIMENSION; x++)
        {
            if (map[y][x] == 0)
            {
                data[m++] = 255; data[m++] = 255; data[m++] = 255;
            }   
            else if (map[y][x] == 1)
            {
                data[m++] = 255; data[m++] = 0; data[m++] = 0; 
            }
            else if (map[y][x] == 2)
            {
                data[m++] = 0; data[m++] = 0; data[m++] = 255; 
            }
                
        }
    }

    image =  Mat(300,300, CV_8UC3, data);
    imwrite("/home/pi/map.jpg", image);
}

Point location(float realHeight, Rect object)
{
    float x, y, diagnol, angle, angle_y;
    

    diagnol = (FOCAL_LENGTH * realHeight * FRAME_HEIGHT) / (object.height * SENSOR_HEIGHT);
    angle = ((object.x + object.width / 2) - (320/2)) * ANGLE_PER_PIXEL_X;
    angle_y = ((object.y + object.height / 2) - (200/2)) * ANGLE_PER_PIXEL_Y;

    //printf("%f\n", angle_y);

    x = diagnol * sin(angle * PI / 180);
    y = diagnol * cos(angle * PI / 180);

    return Point(x,y);
}

Rect getObject()
{
    Mat imageHSV;
    Mat imageRGB;
    Mat mask;
    vector<vector<Point> > contours;
    Mat contourOutput;

    imageHSV = getImage();

    cvtColor (imageHSV,imageRGB,CV_HSV2BGR);
    inRange(imageHSV, Scalar(10,75,10), Scalar(35,180,90),mask);

    int largest_area=0;
    int largest_contour_index=0;
    
    contourOutput = mask.clone();
    findContours( contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE );

    for( int i = 0; i< contours.size(); i++ )  
    {
       double a = contourArea( contours[i], false); 
       if(a > largest_area){
        largest_area = a;
        largest_contour_index = i;               
      }
    }
    
    Rect box = boundingRect(contours[largest_contour_index]);
    Point pt1,pt2;

    pt1.x = box.x;
    pt1.y = box.y;
    pt2.x = box.x + box.width;
    pt2.y = box.y + box.height;
    
    rectangle(imageRGB, pt1, pt2, CV_RGB(255,0,0), 1);
    
    imwrite("/home/pi/x.jpg", imageRGB);
    
    #if 0
    namedWindow( "RGB", CV_WINDOW_NORMAL );
    namedWindow( "HSV", CV_WINDOW_NORMAL );
    namedWindow( "Mask", CV_WINDOW_NORMAL);
    
    imshow("HSV", imageHSV);
    imshow("RGB", imageRGB);
    imshow("Mask", mask);

    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/Mask.jpg", mask);

    waitKey(0);
    setMouseCallback("HSV", mouseEvent, &imageHSV);
    #endif

    return box;
}

Mat getImage()
{
    unsigned char *pixels;
    int32_t response, fourcc;
    int8_t renderflags;
    int return_value, res;
    uint16_t rwidth, rheight;
    uint32_t  numPixels;
    uint16_t height,width;
    uint16_t mode;
    
    return_value = pixy_command("run", END_OUT_ARGS, &response, END_IN_ARGS);   
    return_value = pixy_command("stop", END_OUT_ARGS, &response, END_IN_ARGS);

    return_value = pixy_command("cam_getFrame",  // String id for remote procedure
                                 0x01, 0x21,      // mode
                                 0x02,   0,        // xoffset
                                 0x02,   0,         // yoffset
                                 0x02, 320,       // width
                                 0x02, 200,       // height
                                 0,            // separator
                                 &response, &fourcc, &renderflags, &rwidth, &rheight, &numPixels, &pixels, 0);

    return renderBA81(renderflags,rwidth,rheight,numPixels,pixels);
}

void mouseEvent(int evt, int x, int y, int flags, void* param) 
{                    
    Mat* rgb = (Mat*) param;
    if (evt == CV_EVENT_LBUTTONDOWN) 
    { 
        printf("%d %d: %d, %d, %d\n", 
        x, y, 
        (int)(*rgb).at<Vec3b>(y, x)[0], 
        (int)(*rgb).at<Vec3b>(y, x)[1], 
        (int)(*rgb).at<Vec3b>(y, x)[2]); 

    }         
}

inline void interpolateBayer(uint16_t width, uint16_t x, uint16_t y, uint8_t *pixel, uint8_t* r, uint8_t* g, uint8_t* b)
{
    if (y&1)
    {
        if (x&1)
        {
            *r = *pixel;
            *g = (*(pixel-1)+*(pixel+1)+*(pixel+width)+*(pixel-width))>>2;
            *b = (*(pixel-width-1)+*(pixel-width+1)+*(pixel+width-1)+*(pixel+width+1))>>2;
        }
        else
        {
            *r = (*(pixel-1)+*(pixel+1))>>1;
            *g = *pixel;
            *b = (*(pixel-width)+*(pixel+width))>>1;
        }
    }
    else
    {
        if (x&1)
        {
            *r = (*(pixel-width)+*(pixel+width))>>1;
            *g = *pixel;
            *b = (*(pixel-1)+*(pixel+1))>>1;
        }
        else
        {
            *r = (*(pixel-width-1)+*(pixel-width+1)+*(pixel+width-1)+*(pixel+width+1))>>2;
            *g = (*(pixel-1)+*(pixel+1)+*(pixel+width)+*(pixel-width))>>2;
            *b = *pixel;
        }
    }

}


Mat renderBA81(uint8_t renderFlags, uint16_t width, uint16_t height, uint32_t frameLen, uint8_t *frame)
{
    uint16_t x, y;
    uint8_t r, g, b;
    Mat imageRGB;
    Mat imageHSV;

    frame += width;
    uchar data[3*((height-2)*(width-2))];

    uint m = 0;
    for (y=1; y<height-1; y++)
    {
        frame++;
        for (x=1; x<width-1; x++, frame++)
        {
            interpolateBayer(width, x, y, frame, &r, &g, &b);
            data[m++] = b;
            data[m++] = g;
            data[m++] = r;
        }
        frame++;
    }

    imageRGB =  Mat(height - 2,width -2, CV_8UC3, data);
    
    cvtColor (imageRGB,imageHSV,CV_BGR2HSV);
    
    return imageHSV;
}
