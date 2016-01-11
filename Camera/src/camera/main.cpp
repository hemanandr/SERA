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

#define PI                  3.14159265

#define SERIAL              "/home/pi/SERA/Pipes/p_serial"
#define OUTPUT              "/home/pi/SERA/Pipes/p_output"
#define MOUSE               "/home/pi/SERA/Pipes/p_mouse"

void mouseEvent(int evt, int x, int y, int flags, void* param) ;
Mat getImage();
bool getObject();
bool dropObject();

Point location(float realHeight, Rect object);
Point obstacleLocation(float realHeight, Rect object);
Point destinationLocation(float realHeight, Rect object);

int distance();
void serial(string input);

int serialR();
bool pickObject();
Point mouse();
void ResetMouse();

void turnL();
void turnR();

void moveTo(int x, int y);

vector<Point> obstacles;
vector<Point> objects;
Point destination = Point(999,999);

int angle_offset = 0;
int mouse_x = 0;
int mouse_y = 0;
int direction_x = 0;
int direction_y = 1;

void line();
void newPosition();
void searchDrop();
void searchPick();

void line()
{   
    serial("w");
    serialR();
    
    int dis;

    ResetMouse();

    do
    {
        serial("x");
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 20);

    serial("s");

    ResetMouse();
    
    serial("r");
    serialR();
    serial("r");
    serialR();
    serial("r");
    serialR();
    serial("r");
    serialR();
}

void newPosition()
{
    bool free;

    do
    {
        serial("r");
        serialR();
        free = getObject();
    }while(!free);

    line();
}

void searchPick()
{
    serial("v.150");
    serial("c.100");
    
    bool picked = false;
    int count = 0;
    
    printf("Picking Object\n");

    do
    {
        picked = pickObject();
        serial("t");
        serialR();
        count++;

        if(count > 10)
        {
            newPosition();
            return searchPick();
        }
    }while(!picked);

    return;
}

void searchDrop()
{
    serial("v.150");
    serial("c.100");
    
    bool dropped = false;        
    int count = 0;
 
    do
    {
        printf("Dropping Object\n");
        dropped = dropObject();
        serial("t");
        serialR();
        count++;

        if(count > 10)
        {
            newPosition();
            return searchDrop();
        }
    }while(!dropped); 

    return;
}

int main(int argc, char * argv[])
{
    int count = 0;

    while(1)
    {   
        searchPick();
        searchDrop();
        count++;
    }

    return 0;
}

/*****************************************************************************************************
                                            Pick Object
******************************************************************************************************/
bool pickObject()
{
    Mat imageHSV, imageRGB, red, green, yellow;
    
    vector<vector<Point> > contours, objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);

    inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
    inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,200),green);
        
    Mat combined;
    bitwise_or(red, yellow, combined);
    //bitwise_or(combined, green, combined);
    
    {  
        findContours( combined.clone(), objectContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
        
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
            Point collection = destinationLocation(5,object);

            int min, max;
            
            if(collection.y > 45){
                min = -3;
                max = 1;
            }else{
                min = -2;
                max = 0;
            }

            printf("Object : %d %d\n", collection.x, collection.y);
            if(collection.x > min & collection.x < max){
                int dis;

                if(collection.y < 45){
                    printf("Initialize\n");
                    serial("i");
                    serialR();

                    printf("Pickup\n");
                    serial("p");
                    serialR();

                    return true;
                }

                serial("w");

                do
                {
                    dis = mouse().y;
                    printf("Dest:%d %d\n", collection.y, dis);
                }while(dis < abs(collection.y) - 35);

                serial("s");

                ResetMouse();

                serial("v.170");
                return pickObject();
            }
            else{
                if(collection.x <= min){
                    int temp = collection.x / 5;
                    if(collection.x > -5){
                        turnL();
                    }
                    else{
                        for(int i = 0; i < abs(temp); i++)
                        {
                            turnL();   
                        }
                        usleep(5000);
                    }   
                }
                else if(collection.x >= max){
                    int temp = collection.x / 5;
                    if(collection.x < 5){
                        turnR();
                    }
                    else{
                        for(int i = 0; i < abs(temp); i++)
                        {
                            turnR();   
                        }
                        usleep(5000);
                    } 
                }
                return pickObject();   
            }

        }

    } 

    #if 1
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/Yellow.jpg", yellow);
    imwrite("/home/pi/Green.jpg", green);
    imwrite("/home/pi/Combined.jpg", combined);
    #endif

    imageRGB.release();
    imageHSV.release();
    yellow.release();
    green.release();
    red.release();
    combined.release();

    return false;
}

/*****************************************************************************************************
                                            Drop Object
******************************************************************************************************/
bool dropObject()
{
    serial("v.150");

    Mat imageHSV, imageRGB, white;
    
    vector<vector<Point> > contours, objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
   
    inRange(imageHSV, Scalar(15,0,165), Scalar(100,25,255),white);
    imwrite("/home/pi/White.jpg", white);
      
    {  
        findContours( white.clone(), objectContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
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

            int min, max;
            
            if(collection.y > 60){
                min = -5;
                max = 5;
            }else{
                min = -2;
                max = 2;
            }

            printf("Object : %d %d\n", collection.x, collection.y);
            if(collection.x > min  & collection.x < max){
                int dis;
                
                /*if(collection.y < 60)
                {
                    printf("Drop\n");
                    serial("l");
                    serialR();
                
                    return true;
                }*/

                serial("w");

                do
                {
                    dis = mouse().y;
                    printf("Dest:%d %d\n", collection.y, dis);
                }while(dis < abs(collection.y) - 50);
                
                printf("Drop\n");
                serial("l");
                serialR();
                
                serial("i");
                serialR();
                
                return true;
                
            }
            else{
                if(collection.x <= min){
                    int temp = collection.x / 5;
                    if(collection.x > -5){
                        turnL();
                    }
                    else{
                        for(int i = 0; i < abs(temp); i++)
                        {
                            turnL();   
                        }
                        usleep(5000);
                    }   
                }
                else if(collection.x >= max){
                    int temp = collection.x / 5;
                    if(collection.x < 3){
                        turnR();
                    }
                    else{
                        for(int i = 0; i < abs(temp); i++)
                        {
                            turnR();   
                        }
                        usleep(5000);
                    } 
                }
                return dropObject();   
            }
        }
    } 



    #if 1
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/Yellow.jpg", white);
    #endif

    imageRGB.release();
    imageHSV.release();
    white.release();

    return false;
}

/*****************************************************************************************************
                                            Get Object
******************************************************************************************************/
bool getObject()
{
    bool free = true;

    Mat imageHSV, imageRGB;
    Mat white, blue, black, gray;
    Mat contourOutput;
    
    Mat yellow, red, green;

    vector<vector<Point> > contours;
    vector<vector<Point> > objectContours;
    vector<Point> fieldContour;

    int largest_area;
    int largest_contour_index=0;

    imageHSV = getImage();
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
   
    inRange(imageHSV, Scalar(90,60,180), Scalar(105,220,255),blue);
    inRange(imageHSV, Scalar(15,0,165), Scalar(100,25,255),white);
    inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,200),green);
    inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
    inRange(imageHSV, Scalar(0,0,0), Scalar(180,190,90),black);
    inRange(imageHSV, Scalar(80,20,150), Scalar(120,100,255),gray);

    if 1
    namedWindow( "RGB", CV_WINDOW_NORMAL );
    namedWindow( "HSV", CV_WINDOW_NORMAL );

    namedWindow( "1", CV_WINDOW_NORMAL );
    namedWindow( "2", CV_WINDOW_NORMAL );

    imshow("HSV", imageHSV);
    imshow("RGB", imageRGB);
    imshow("1", red);
    imshow("2", yellow);

    setMouseCallback("RGB", mouseEvent, &imageHSV);
    waitKey(0);
    #endif

    //Detect the game field
    contourOutput = blue.clone();
    findContours( contourOutput, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
    
    largest_area = 0;
    largest_contour_index = -1;

    for( int i = 0; i< contours.size(); i++ )  
    {
       double a = contourArea( contours[i], false);
       if(a > largest_area){
             largest_area = a;
             largest_contour_index = i;
      }
    }
    
    //Draw the Field Contour
    if(largest_contour_index != -1)
        fieldContour = contours[largest_contour_index];


/*****************************************************************************************************
                                        Obstacle Detection
******************************************************************************************************/
#if 1
{
    Mat combined;
    bitwise_or(black, gray, combined);

    
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
            drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,0), -1, 8);
            Rect object = boundingRect(objectContours[largest_contour_index]);
            Point obstacle = obstacleLocation(38,object);
            printf("Obstacle : %d %d\n", obstacle.x, obstacle.y);
            if(obstacle.x > -20 & obstacle.x < 20)
            {
                free = false;
            }    
            //objects.push_back(Point(collection.x, collection.y));
        }
        else
        {
            break;
        }
    }  
    combined.release(); 
} 
#endif

    #if 0
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/White.jpg", white);
    imwrite("/home/pi/Blue.jpg", blue);
    imwrite("/home/pi/Black.jpg", black);
    imwrite("/home/pi/Gray.jpg", gray);
    #endif

    imageHSV.release();
    imageRGB.release();
    blue.release();
    black.release();
    gray.release();
    contourOutput.release();

    return free;
}

/*****************************************************************************************************
                                        Location Algorithm
******************************************************************************************************/
Point location(float realHeight, Rect object)
{
    float x, y, diagnol, angle;
        
    diagnol = (FOCAL_LENGTH * realHeight * FRAME_HEIGHT) / (object.height * SENSOR_HEIGHT);
    angle = (((object.x + object.width / 2) - (FRAME_WIDTH/2)) * ANGLE_PER_PIXEL_X) + angle_offset;

    x = diagnol * sin(angle * PI / 180) * direction_y;
    y = (diagnol * cos(angle * PI / 180)) * direction_y;

    return Point(x,y);
}

Point obstacleLocation(float realHeight, Rect object)
{
    float x, y, diagnol, angle;
        
    diagnol = (FOCAL_LENGTH * realHeight * FRAME_HEIGHT) / (object.height * SENSOR_HEIGHT);
    angle = (((object.x + object.width) - (FRAME_WIDTH/2)) * ANGLE_PER_PIXEL_X) + angle_offset;

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
//  serial('q');

    FILE* fp;
    char readbuf[10];
    int distance;

    fp = fopen(OUTPUT, "r");
    fgets(readbuf, 10, fp);
    fclose(fp);

    distance = atoi(readbuf);
    
    return distance;
}

/*****************************************************************************************************
                            Serial Communication - Write and Read - Blocking
******************************************************************************************************/    
void serial(string input)
{
    FILE* fp;
    fp = fopen(SERIAL, "w");
    fputs(&input[0], fp);
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


/*****************************************************************************************************
                                            Mouse Access Modules
******************************************************************************************************/  
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

void ResetMouse(){
    Point m = mouse();

    mouse_x += m.x;
    mouse_y += m.y;
}

/*****************************************************************************************************
                                            Serial Navigation
******************************************************************************************************/
void turnR(){
        serial("m");
}

void turnL(){
        serial("n");
}

void moveTo(int x, int y)
{
    //ResetMouse();
    printf("Move To %d %d\n", x, y);
    int dis, temp;
    if(direction_y == 1)
    {
        if(x < 0)
        {
            serial("r");
            direction_x = -1;
            direction_y = 0;   
        }else if (x > 0)
        {
            serial("t");
            direction_x = 1;
            direction_y = 0; 
        }
    }else if(direction_y == -1)
    {
        if(x < 0)
        {
            serial("t");
            direction_x = 1;
            direction_y = 0; 
        }else if (x > 0)
        {
            serial("r");
            direction_x = -1;
            direction_y = 0; 
        }
    }else if (direction_y == 0)
    {
        if(direction_x == 1)
        {
            serial("tt");
            direction_x = -1;
        }
    }

    while(serialR() != 1){}

    do
    {
        serial("w");
        dis = mouse().y;
        printf("%d %d\n", dis, x );
    }while(dis < abs(x));

    serial("s");
    ResetMouse();

    if(y > 0)
    {
        if(direction_x == 1)
        {
            serial("r");
        }else if(direction_x == -1)
        {
            serial("t");
        }
    }else if(y<0)
    {
        if(direction_x == 1)
        {
            serial("r");
        }else if(direction_x == -1)
        {
            serial("t");
        }
    }

    while(serialR() != 1){}

    do
    {
        serial("w");
        dis = mouse().y;
        printf("%d %d\n", dis, x );
    }while(dis < abs(y));

    serial("s");
    ResetMouse();

}

/*****************************************************************************************************
                                        Image Acquisition and Calibration Services
******************************************************************************************************/
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