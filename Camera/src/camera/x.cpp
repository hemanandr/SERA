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

void mouseEvent(int evt, int x, int y, int flags, void* param) ;
Mat getImage();
void getObject();
bool dropObject();

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

void serial(string input)
{
    FILE* fp;
    fp = fopen(SERIAL, "w");
    fputs(&input[0], fp);
    fclose(fp);
}

int main(int argc, char * argv[])
{
        serial("c.180.");
        angle_offset = -90;
        getObject();

        serial("c.140.");
        angle_offset = -45;
        getObject();

        serial("c.100.");
        angle_offset = 0;
        getObject();

        serial("c.60.");
        angle_offset = 45;
        getObject();

        serial("c.20.");
        angle_offset = 90;
        getObject();

        serial("rr");
        direction_y = -1;

         serial("c.180.");
        angle_offset = -90;
        getObject();

        serial("c.140.");
        angle_offset = -45;
        getObject();

        serial("c.100.");
        angle_offset = 0;
        getObject();

        serial("c.60.");
        angle_offset = 45;
        getObject();

        serial("c.20.");
        angle_offset = 90;
        getObject();
    

    //bool fr;
    //bool grip;
    //fr = getObject();
    
    /*int my;
    serial('w');
    do
    {
        my = mouse().y;
        printf("%d\n",my );
    }while(my < 135);
    serial('s');
    usleep(5000);
                    
    serial('m');
    serial('t');
    serial('t');

    mouse_y = my;


    fr = getObject();
    
    while(objects.size() == 0)
    {
        serial('t');
        fr = getObject();
    }

    while(1)
    {
        grip = gripObject();
    }
*/
    return 0;

}

bool gripObject()
{
    Mat imageHSV;
    Mat imageRGB;
    Mat green, yellow, blue, red;
    Mat contourOutput, output;
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

/*
        if(largest_contour_index != -1)
        {
            drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,255), 1, 8);
            Rect object = boundingRect(objectContours[largest_contour_index]);
            Point collection = destinationLocation(5,object);

            printf("Object : %d %d\n", collection.x, collection.y);
            if(collection.x > -4  & collection.x < 5){
                int my, mx;
                
                if(collection.y < 40)
                    serial("i");

                if(collection.y < 20)
                {

                    serial('s');
                    usleep(5000);
                    serial('p');
                    usleep(5000);
                    serial('m');
                    while(1){
                        bool x = dropObject;
                    }
                }

                serial('w');
                usleep(5000);
                
                do
                {
                    my = mouse().y;
                    printf("Mouse Y : %d Offset : %d\n", my, mouse_y );
                }while(my < abs(collection.y) - 50);
                serial('s');

                mouse_y = my;

                return gripObject();
            }
            else{
                if(collection.x < 0){
                    turnL();   
                }
                else if(collection.x > 0){
                    turnR();     
                }
                return gripObject();   
            }

        }
*/
    } 



    #if 0
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/Yellow.jpg", yellow);
    imwrite("/home/pi/Green.jpg", green);
    #endif

    return free;
}

bool dropObject()
{
    Mat imageHSV;
    Mat imageRGB;
    Mat white;
    Mat contourOutput, output;
    Rect x;
    bool free = true;

    vector<vector<Point> > contours;
    vector<vector<Point> > objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
   
    inRange(imageHSV, Scalar(15,0,165), Scalar(100,25,255),white);

        
    Mat combined = white.clone();
    
    {  
        findContours( combined.clone(), objectContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
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
            drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,255), 1, 8);
            Rect object = boundingRect(objectContours[largest_contour_index]);
            Point collection = destinationLocation(30,object);

            printf("Object : %d %d\n", collection.x, collection.y);
            if(collection.x > -6  & collection.x < 6){
                int my, mx;
                
                if(collection.y < 10)
                {
                    serial("s");
                    usleep(5000);
                    serial("l");
                    usleep(5000);
                    exit(0);
                }
                serial("w");
                do
                {
                    my = mouse().y;
                    printf("Mouse Y : %d Offset : %d\n", my, mouse_y );
                }while(my < abs(collection.y) - 50);
                serial("s");

                mouse_y = my;

                return true;
            }
            else{
                if(collection.x < 0){
                    turnL();   
                }
                else if(collection.x > 0){
                    turnR();     
                }
                return dropObject();   
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
        serial("t");
}

void turnL(){
        serial("r");
}

void getObject()
{
    Mat imageHSV;
    Mat white, blue, dblue, red, yellow, black, green, gray;
    Mat contourOutput;
    
    vector<vector<Point> > contours;
    vector<vector<Point> > objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    
    inRange(imageHSV, Scalar(90,60,180), Scalar(105,220,255),blue);
    inRange(imageHSV, Scalar(15,0,165), Scalar(100,25,255),white);
    inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,200),green);
    inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
    inRange(imageHSV, Scalar(0,0,0), Scalar(180,190,90),black);
    inRange(imageHSV, Scalar(80,20,150), Scalar(120,100,255),gray);

    //Detect the game field
    contourOutput = blue.clone();
    findContours( contourOutput, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
    blue.release();
    
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
    //drawContours( imageRGB, contours, largest_contour_index, Scalar(0,255,0), 3, 8);
    fieldContour = contours[largest_contour_index];

/*****************************************************************************************************
                                Check for Collection Point
******************************************************************************************************/
if(0)
{  
    findContours( white.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
    white.release();
    
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
if(0)
{
    Mat combined;
    bitwise_or(green, yellow, combined);
    bitwise_or(yellow, red, combined);
    
    red.release();
    yellow.release();
    green.release();
    
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

        //drawContours( imageRGB, objectContours, -1, Scalar(0,0,255), 1, 8);

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
            //drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,0), -1, 8);
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

    combined.release(); 
} 

/*****************************************************************************************************
                                        Obstacle Detection
******************************************************************************************************/
if(0)
{
    Mat combined;
    bitwise_or(black, gray, combined);

    black.release();
    gray.release();
    
    while(1)
    {
        objectContours.clear();
        findContours( combined.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
    
        combined.release();
    
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

    contourOutput.release();
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
//    serial('q');

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
    system("sudo rm /home/pi/image.jpg");
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