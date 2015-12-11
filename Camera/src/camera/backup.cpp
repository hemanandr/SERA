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

#define NORTH   0
#define SOUTH   1
#define EAST    2
#define WEST    3

#define FOCAL_LENGTH        0.28
#define FRAME_HEIGHT        200
#define FRAME_WIDTH         320
#define SENSOR_HEIGHT       0.243
#define SENSOR_WIDTH        0.3888
#define ANGLE_PER_PIXEL_X   75.0/320.0
#define ANGLE_PER_PIXEL_Y   47.0/200.0

#define PI              3.14159265

#define FIELD_DIMENSION 300
#define SERIAL          "/home/pi/SERA/Pipes/p_serial"
#define OUTPUT          "/home/pi/SERA/Pipes/p_output"

int direction = NORTH;
uint8_t botx = 150, boty = 30;

void interpolateBayer(unsigned int width, unsigned int x, unsigned int y, unsigned char *pixel, unsigned int &r, unsigned int &g, unsigned int &b);
Mat renderBA81(uint8_t renderFlags, uint16_t width, uint16_t height, uint32_t frameLen, uint8_t *frame);
void mouseEvent(int evt, int x, int y, int flags, void* param) ;
Mat getImage();
Rect getObject(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION]);
Point destLocation(float realWidth, Rect object);

Point location(float realHeight, Rect object);
void printMap(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION]);
void mapBot(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION]);
void mapObject(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION], int8_t x, int8_t y);
void serial(char input);
int distance();

    
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
            if(x%2)
                map[y][x] = 0;
            else
                map[y][x] = 0;

        }
    }

    mapBot(map);

    
    Rect object;
    
    {
        Point objectLocation;
        time_t now = time(0);
        char* dt = ctime(&now);
        object = getObject(map);
    }
    

    pixy_close();
    exit(0);
  
}

Rect getObject(uint8_t map[FIELD_DIMENSION][FIELD_DIMENSION])
{
    Mat imageHSV;
    Mat imageRGB;
    Mat white, blue, dblue, red, yellow, black;
    Mat contourOutput, temp, output;
    
    vector<vector<Point> > contours;
    vector<vector<Point> > objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;
    int top = 200;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
   
    //Adjusted On 25/11/2015
    inRange(imageHSV, Scalar(105,70,25), Scalar(120,120,45),dblue);
    inRange(imageHSV, Scalar(20,5,120), Scalar(65,60,255),white);
    inRange(imageHSV, Scalar(30,5,80), Scalar(105,100,160),blue);
    inRange(imageHSV, Scalar(20,140,130), Scalar(30,170,175),yellow);
    inRange(imageHSV, Scalar(0,150,0), Scalar(4,200,255),red);
    inRange(imageHSV, Scalar(0,0,0), Scalar(0,0,25),black);

    //Detect the game field
    contourOutput = blue.clone();
    findContours( contourOutput, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
    
    largest_area = 0;
    for( int i = 0; i< contours.size(); i++ )  
    {
       double a = contourArea( contours[i], false);
       if(a > largest_area){
             largest_area = a;
             largest_contour_index = i;
      }
    }
    
    //Draw the Field Contour
    drawContours( imageRGB, contours, largest_contour_index, Scalar(255,0,0), 1, 8);
    fieldContour = contours[largest_contour_index];

    //Edge detection on the HSV Image
    Canny( imageHSV, imageHSV, 100, 250, 3);
    contourOutput = imageHSV.clone();
    findContours( contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
    
    //Check if detected contour is within the Field Contour
    for(int i = 0; i < contours.size(); i++)
    {
        for(int j = 0; j < contours[i].size(); j++)
        {
            double test = pointPolygonTest(fieldContour, contours[i][j], false);   
            if(test > -1)
            {
                objectContours.push_back(contours[i]);
                break;
            }
        }
    }
    
    //Destination Point Detection
    {
        int width = 0;
        float min_pixel_ratio = 100;
        Rect object;
        RotatedRect minEllipse;
            
        for( int i = 0; i < objectContours.size(); i++ ){  
            float pixel_ratio = 0.0;

            Mat image(198, 318, CV_8UC3, Scalar(0));
            
            if(objectContours[i].size() > 5)
            {
                minEllipse = fitEllipse( Mat(objectContours[i]) );
            
                ellipse(image, minEllipse, Scalar(255,255,255), -1);
                inRange(image,Scalar(255,255,255),Scalar(255,255,255),image);

                bitwise_and(image,white,temp);

                int x = countNonZero(image);
                int y = countNonZero(temp);
                pixel_ratio = (float)x / (float)y;
                        
                if((pixel_ratio < min_pixel_ratio)){
                    min_pixel_ratio = pixel_ratio;
                    ellipse( imageRGB, minEllipse, Scalar(0,0,255), 1);
                    object = boundingRect(objectContours[i]);
                }
            }
        }

        Point collection = destLocation(30,object);
    
        if((collection.y > 0) & (collection.y < 200))
        {
            printf("%d %d\n", collection.x, collection.y);
            mapObject(map, collection.x, collection.y);
        }
    }

   
    


    #if 1
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/White.jpg", white);
    imwrite("/home/pi/Blue.jpg", blue);
    imwrite("/home/pi/DBlue.jpg", dblue);
    imwrite("/home/pi/Red.jpg", red);
    imwrite("/home/pi/Yellow.jpg", yellow);
    imwrite("/home/pi/Black.jpg", black);
    #endif

    #if 0
    namedWindow( "RGB", CV_WINDOW_NORMAL );
    namedWindow( "HSV", CV_WINDOW_NORMAL );
    namedWindow( "Mask", CV_WINDOW_NORMAL);
    
    imshow("HSV", xa);
    imshow("RGB", imageRGB);
    imshow("Mask", blue);

    setMouseCallback("Mask", mouseEvent, &xa);
    waitKey(0);
    #endif


    printMap(map);

    Rect z = boundingRect(contours[largest_contour_index]);
    
    return z;
}

Point destLocation(float realWidth, Rect object)
{
    float x, y, diagnol, angle, angle_y;
    
    diagnol = (FOCAL_LENGTH * realWidth * FRAME_WIDTH) / (object.width * SENSOR_WIDTH);
    angle = ((object.x + object.width / 2) - (320/2)) * ANGLE_PER_PIXEL_X;
    angle_y = ((object.y + object.height / 2) - (200/2)) * ANGLE_PER_PIXEL_Y;

    x = diagnol * sin(angle * PI / 180);
    y = diagnol * cos(angle * PI / 180);
   
    return Point(x,y);
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
    uint8_t length = 30;
    uint8_t width = 30;

    uint8_t y_start, y_end, x_start, x_end;

    if(direction == NORTH){
        y_start = boty - length/2 + y;
        y_end = boty + length/2 + y;

        x_start = botx - width/2 + x;
        x_end = botx + width/2 + x;
    }


    printf("X %d %d %d\n", botx, width/2, x);
    printf("X %d %d\n", x_start, x_end);
    printf("Y %d %d\n", y_start, y_end);

    for(int i = y_start; i < y_end; i++)
    {
        for(int j = x_start; j < x_end; j++)
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
                data[m++] = 0; data[m++] = 0; data[m++] = 0; 
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























int distance(){
    serial('q');

    FILE* fp;
    char readbuf[10];
    int distance;

    fp = fopen(OUTPUT, "r");
    fgets(readbuf, 10, fp);
    fclose(fp);


    distance = atoi(readbuf);
    
    return distance;
}

void serial(char input)
{
    FILE* fp;
    fp = fopen(SERIAL, "w");
    fputs(&input, fp);
    fclose(fp);
}







Mat getImage()
{
   

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