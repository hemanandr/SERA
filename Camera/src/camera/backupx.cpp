#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <math.h> 
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <ctime>

using namespace cv;
using namespace std;

#define FOCAL_LENGTH        0.36
#define FRAME_HEIGHT        768
#define FRAME_WIDTH         1024
#define SENSOR_HEIGHT       0.2738
#define SENSOR_WIDTH        0.3673
#define ANGLE_PER_PIXEL_X   53.5/1024.0
#define ANGLE_PER_PIXEL_Y   41.41/768.0

#define PI              3.14159265

#define FIELD_DIMENSION 300
#define SERIAL          "/home/pi/SERA/Pipes/p_serial"
#define OUTPUT          "/home/pi/SERA/Pipes/p_output"
#define MOUSE          "/home/pi/SERA/Pipes/p_mouse"
#define MOUSE_I   "/home/pi/SERA/Pipes/p_mousei"

void interpolateBayer(unsigned int width, unsigned int x, unsigned int y, unsigned char *pixel, unsigned int &r, unsigned int &g, unsigned int &b);
Mat renderBA81(uint8_t renderFlags, uint16_t width, uint16_t height, uint32_t frameLen, uint8_t *frame);
void mouseEvent(int evt, int x, int y, int flags, void* param) ;
Mat getImage();
bool getObject();

Point location(float realHeight, Rect object);
Point destinationLocation(float realHeight, Rect object);

int distance();
void serial(char input);

int serialR();
bool gripObject();
Point mouse();
void turnL();
void turnR();

void moveTo(int x, int y);

vector<Point> obstacles;
vector<Point> objects;
Point destination = Point(999,999);

int angle_offset = 0;
int mouse_x = 0;
int mouse_y = 0;
int direction_x = 1;
int direction_y = 1;

int main(int argc, char * argv[])
{
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

    {
        bool free;

        serial('c');
        serial('.');
        serial('1');
        serial('8');
        serial('0');
        serial('.');
        angle_offset = -90;
        free = getObject();

        serial('c');
        serial('.');
        serial('1');
        serial('4');
        serial('0');
        serial('.');
        angle_offset = -45;
        free = getObject();

        serial('c');
        serial('.');
        serial('1');
        serial('0');
        serial('0');
        serial('.');
        angle_offset = 0;
        free = getObject();

        serial('c');
        serial('.');
        serial('6');
        serial('0');
        serial('.');
        angle_offset = 45;
        free = getObject();

        serial('c');
        serial('.');
        serial('2');
        serial('0');
        serial('.');
        angle_offset = 90;
        free = getObject();

        /*
        serial('n');
        serial('n');
        direction_y = -1;

        serial('c');
        serial('.');
        serial('1');
        serial('8');
        serial('0');
        serial('.');
        angle_offset = -90;
        free = getObject();

        serial('c');
        serial('.');
        serial('1');
        serial('4');
        serial('0');
        serial('.');
        angle_offset = -45;
        free = getObject();

        serial('c');
        serial('.');
        serial('1');
        serial('0');
        serial('0');
        serial('.');
        angle_offset = 0;
        free = getObject();


        serial('c');
        serial('.');
        serial('6');
        serial('0');
        serial('.');
        angle_offset = 45;
        free = getObject();

        serial('c');
        serial('.');
        serial('2');
        serial('0');
        serial('.');
        angle_offset = 90;
        free = getObject();*/
       
    }

    {
        for( int i = 0; i < 1; i++ )  
        {  
            int x = objects[i].x;
            int y = objects[i].y;

            moveTo(objects[i].x, objects[i].y);
           

            serial('c');
            serial('.');
            serial('1');
            serial('0');
            serial('0');
            serial('.');
            angle_offset = 0;

            bool grip = gripObject();

            //serial('i');
            //serial('w');
            
            int my, mx;
            do
            {
                my = mouse().y;
                mx = mouse().x;
            }while(my < abs(y) - 10);

            serial('s'); 

            mouse_y = my;
            mouse_x = mx;
            
            grip = gripObject();

            //serial('p');

            //serial('n');
            //serial('n');
        }

    }
    
    return 0;
}

void moveTo(int x, int y)
{
           if(y > 0)
           {
            if(direction_y == -1)
            {
                if(x < 0)
                {
                    serial('m');
                    usleep(5000);
                    direction_x = -1;
                    direction_y = 0;
                }else
                {
                    serial('n');
                    usleep(5000);
                    direction_x = 1;
                    direction_y = 0;
                } 
            }else if(direction_y == 1)
            {
                if(x > 0)
                {   
                    serial('m');
                    usleep(5000);
                    direction_x = 1;
                    direction_y = 0;
                }else{
                    serial('n');
                    getchar();
                    usleep(5000);
                    direction_x = -1;
                    direction_y = 0;
                }
            }

           }else if(y < 0)
           {
            if(direction_y == 1)
            {
               if(x > 0)
                {
                    serial('m');
                    usleep(5000);
                    direction_x = 1;
                    direction_y = 0;
                }else
                {
                    serial('n'); 
                    usleep(5000);                   
                    direction_x = -1;
                    direction_y = 0;
                } 
            }else if(direction_y == -1)
            {
                if(x > 0)
                {   
                    serial('n');
                    usleep(5000);
                    direction_x = 1;
                    direction_y = 0;
                }else{
                    serial('m');
                    usleep(5000);         
                    direction_x = -1;
                    direction_y = 0;
                }
            }
            }  

            serial('w');
                
            int my,mx;
            do
            {
                my = mouse().y;
                mx = mouse().x;
            }while(my < abs(x));

            serial('s'); 
            usleep(5000);

            //mouse_y = my;
            //mouse_x = mx;
            
            if(x < 0)
            {
                serial('m');
            }else
            {
                serial('n');
            }
}

bool gripObject()
{
    Mat imageHSV;
    Mat imageRGB;
    Mat green, yellow, blue, red;
    Mat contourOutput, temp, output;
    Rect x;
    bool free = true;

    vector<vector<Point> > contours;
    vector<vector<Point> > objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
   

    inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,200),green);
    inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
        
    Mat combined;// = yellow.clone();
    bitwise_or(red, yellow, combined);
    //bitwise_or(combined, green, combined);
    

    {  
        findContours( combined.clone(), objectContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
        drawContours( imageRGB, objectContours, -1, Scalar(0,0,255), 1, 8);


        imageHSV.release();
        imageRGB.release();
        green.release();
        yellow.release();
        red.release();
        combined.release();


        largest_area = 0;
        largest_contour_index = -1;

        for( int i = 0; i< objectContours.size(); i++ )  
        {
           double a = contourArea( objectContours[i], false);
           if(a > largest_area){
                 largest_area = a;
                 largest_contour_index = i;
          }
        }

        printf("%d\n", largest_contour_index);
        
        if(largest_contour_index != -1)
        {
            drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,255), 1, 8);
            Rect object = boundingRect(objectContours[largest_contour_index]);
            Point collection = destinationLocation(5,object);

            printf("Object : %d %d\n", collection.x, collection.y);
            if(collection.x > -1  & collection.x < 1){
                return true;
            }
            else{
                if(collection.x < -1){
                    turnL();   
                    if(collection.x < -3)
                        turnL();
                }
                else if(collection.x > -1){
                    turnR();   
                    if(collection.x > 3)
                        turnR();   
                }
                return gripObject();   
            }
        }
    } 



    #if 0
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/Yellow.jpg", yellow);
    imwrite("/home/pi/Green.jpg", green);
    #endif

    return free;
}

void turnR(){
        serial('t');
}

void turnL(){
        serial('r');
}

bool getObject()
{
    Mat imageHSV;
    Mat imageRGB;
    Mat white, blue, dblue, red, yellow, black, green, gray;
    Mat contourOutput, temp, output;
    Rect x;

    vector<vector<Point> > contours;
    vector<vector<Point> > objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
   

    inRange(imageHSV, Scalar(90,60,180), Scalar(105,220,255),blue);
    inRange(imageHSV, Scalar(15,0,165), Scalar(100,25,255),white);

    inRange(imageHSV, Scalar(80,20,150), Scalar(120,100,255),gray);

    inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,200),green);
    inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
    
    inRange(imageHSV, Scalar(0,0,0), Scalar(180,190,90),black);

    #if 0
    namedWindow( "RGB", CV_WINDOW_NORMAL );
    namedWindow( "HSV", CV_WINDOW_NORMAL );

    namedWindow( "X", CV_WINDOW_NORMAL );
//  namedWindow( "Y", CV_WINDOW_NORMAL );

    imshow("HSV", imageHSV);
    imshow("RGB", imageRGB);
    imshow("X", green);
//    imshow("Y", gray);

    setMouseCallback("RGB", mouseEvent, &imageHSV);
    //waitKey(0);
    #endif

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
    drawContours( imageRGB, contours, largest_contour_index, Scalar(0,255,0), 3, 8);
    fieldContour = contours[largest_contour_index];

/*****************************************************************************************************
                                Check for Collection Point
******************************************************************************************************/
if(0)
{  
    findContours( white.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
    
    //Check if detected contour is within the Field Contour
    for(int i = 0; i < contours.size(); i++)
    {
        for(int j = 0; j < contours[i].size(); j++)
        {
            double test = pointPolygonTest(fieldContour, contours[i][j], true);  
            if(test > -5)
            {
                if(contourArea( contours[i], false) > 15.0){
                    objectContours.push_back(contours[i]);
                    break;
                }
            }
        }
    }
    
    drawContours( imageRGB, objectContours, -1, Scalar(0,0,255), 1, 8);

    largest_area = 0;
    largest_contour_index = -1;

    for( int i = 0; i< objectContours.size(); i++ )  
    {
       double a = contourArea( objectContours[i], false);
       if(a > largest_area){
             largest_area = a;
             largest_contour_index = i;
      }
    }
    
    if(largest_contour_index != -1)
    {
        Rect object = boundingRect(objectContours[largest_contour_index]);
        Point collection = destinationLocation(30,object);
        printf("Destination : %d %d\n", collection.x, collection.y);
        destination.x = collection.x;
        destination.y = collection.y;
    }
}    

/*****************************************************************************************************
                                            Object Detection
******************************************************************************************************/
if(1)
{
    Mat combined;
    //bitwise_or(green, yellow, combined);
    bitwise_or(yellow, red, combined);
        
    while(1)
    {  
        objectContours.clear();
        findContours( combined.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
        
        //Check if detected contour is within the Field Contour
        for(int i = 0; i < contours.size(); i++)
        {
            for(int j = 0; j < contours[i].size(); j++)
            {
                double test = pointPolygonTest(fieldContour, contours[i][j], true);  
                if(test > -5)
                {
                    if(contourArea( contours[i], false) > 15.0){
                        objectContours.push_back(contours[i]);
                        break;
                    }
                }
            }
        }

        drawContours( imageRGB, objectContours, -1, Scalar(0,0,255), 1, 8);

        largest_area = 0;
        largest_contour_index = -1;

        for( int i = 0; i< objectContours.size(); i++ )  
        {
           double a = contourArea( objectContours[i], false);
           if(a > largest_area){
                 largest_area = a;
                 largest_contour_index = i;
          }
        }
        
        if(largest_contour_index != -1)
        {
            drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,0), -1, 8);
            Rect object = boundingRect(objectContours[largest_contour_index]);

            if(object.x < 50| object.x > 975)
                continue;

            Point collection = location(20,object);
            objects.push_back(Point(collection.x, collection.y));
            printf("Object : %d %d\n", collection.x, collection.y);
        }
        else
        {
            break;
        }
    }   
} 

/*****************************************************************************************************
                                        Obstacle Detection
******************************************************************************************************/
if(0)
{
    Mat combined;
    bitwise_or(black, gray, combined);

    while(1)
    {
        imwrite("/home/pi/Obstacle.jpg", combined);

        objectContours.clear();
        findContours( combined.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
        
        //Check if detected contour is within the Field Contour
        for(int i = 0; i < contours.size(); i++)
        {
            for(int j = 0; j < contours[i].size(); j++)
            {
                double test = pointPolygonTest(fieldContour, contours[i][j], true);  
                if(test > -5)
                {
                    if(contourArea( contours[i], false) > 4000.0){
                        objectContours.push_back(contours[i]);
                        break;
                    }
                }
            }
        }

        drawContours( imageRGB, objectContours, -1, Scalar(0,0,255), 1, 8);

        largest_area = 0;
        largest_contour_index = -1;

        for( int i = 0; i< objectContours.size(); i++ )  
        {
           double a = contourArea( objectContours[i], false);
           if(a > largest_area){
                 largest_area = a;
                 largest_contour_index = i;
          }
        }
        
        if(largest_contour_index != -1)
        {
            drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,0), -1, 8);
            Rect object = boundingRect(objectContours[largest_contour_index]);
            Point collection = location(38,object);
            printf("Obstacle : %d %d\n", collection.x, collection.y);
            objects.push_back(Point(collection.x, collection.y));
        }
        else
        {
            break;
        }
    }   
} 

    imageRGB.release();
    imageHSV.release();
    white.release();
    blue.release();
    dblue.release();
    red.release();
    yellow.release();
    black.release();
    gray.release();
    contourOutput.release();

    temp.release();
    output.release();

    #if 0
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/White.jpg", white);
    imwrite("/home/pi/Blue.jpg", blue);
    imwrite("/home/pi/DBlue.jpg", dblue);
    imwrite("/home/pi/Red.jpg", red);
    imwrite("/home/pi/Yellow.jpg", yellow);
    imwrite("/home/pi/Black.jpg", black);
    imwrite("/home/pi/Gray.jpg", gray);
    #endif

    return true;
}

/*****************************************************************************************************
                                    Location Algorithm
******************************************************************************************************/
Point location(float realHeight, Rect object)
{
    float x, y, diagnol, angle;
    
    //diagnol = (FOCAL_LENGTH * realWidth * FRAME_WIDTH) / (object.width * SENSOR_WIDTH);
    diagnol = (FOCAL_LENGTH * realHeight * FRAME_HEIGHT) / (object.height * SENSOR_HEIGHT);
    angle = (((object.x + object.width / 2) - (FRAME_WIDTH/2)) * ANGLE_PER_PIXEL_X) + angle_offset;

    x = diagnol * sin(angle * PI / 180) * direction_y;
    y = (diagnol * cos(angle * PI / 180)) * direction_y;

    return Point(x,y);
}

Point destinationLocation(float realWidth, Rect object)
{
    float x, y, diagnol, angle;
    
    diagnol = (FOCAL_LENGTH * realWidth * FRAME_WIDTH) / (object.width * SENSOR_WIDTH);
    angle = (((object.x + object.width / 2) - (FRAME_WIDTH/2)) * ANGLE_PER_PIXEL_X) + angle_offset;

    x = diagnol * sin(angle * PI / 180) * direction_y;
    y = (diagnol * cos(angle * PI / 180)) * direction_y;

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

Point mouse(){
    FILE* fp;
    char readbuf[10];
    int distance;

    fp = fopen(MOUSE, "r");
    fgets(readbuf, 10, fp);
    fclose(fp);

    char *pch;

    pch = strtok(readbuf," ");
    int x = atoi(pch);
    pch = strtok (NULL, " ");
    int y = atoi(pch);
    
    return Point(x - mouse_x, y - mouse_y);
}

void serial(char input)
{
    FILE* fp;
    fp = fopen(SERIAL, "w");
    fputs(&input, fp);
    fclose(fp);
}

int serialR()
{   
    char readbuf[10];
    int value;

    FILE* fp;
    fp = fopen(OUTPUT, "r");
    fputs(readbuf, fp);
    fclose(fp);

    value = atoi(readbuf);

    return value;
}


Mat getImage()
{
    Mat imageRGB, imageHSV;
    system("sudo python /home/pi/pic.py");
    imageRGB = imread("/home/pi/image.jpg", CV_LOAD_IMAGE_COLOR);

    cvtColor (imageRGB,imageHSV,CV_BGR2HSV);
    
    return imageHSV;
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