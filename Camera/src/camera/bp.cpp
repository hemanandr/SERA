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

#define DEBUG

/*****************************************************************************************************
                                            Function Definitions
******************************************************************************************************/
//Image Acquisition
Mat getImage();
void mouseEvent(int evt, int x, int y, int flags, void* param) ;

//Robot Action
void newPosition();
void searchDrop();
void searchPick();
    void reachLine();
    bool getObject();
    bool pickObject();
    bool dropObject();   
    void moveTo(int x, int y);
        void turnL();
        void turnR();

//Location Function Definitions
Point location(float realHeight, Rect object);
Point obstacleLocation(float realHeight, Rect object);
Point destinationLocation(float realHeight, Rect object);

//Serial Communincation
void serial(string input);
int serialR();

//Mouse Access
Point mouse();
void ResetMouse();

/*****************************************************************************************************
                                    Global Variables Definitions
******************************************************************************************************/
vector<Point> obstacles;
vector<Point> objects;
Point destination = Point(999,999);

int angle_offset = 0;
int mouse_x = 0;
int mouse_y = 0;
int direction_x = 0;
int direction_y = 1;

int main(int argc, char * argv[])
{   serial("v.170.");
    serial("c.100.");
    usleep(500000);

    int dis;
    bool picked;
    bool dropped;
    
    do{
        picked = pickObject();
        printf("%d\n", picked);
    }while(!picked);    
    printf("Picked Up\n");
    
    serial("r");
    usleep(500000);
    serial("r");
    usleep(500000);
    ResetMouse();

    printf("Drop Started\n");
    do
    {
        dropped = dropObject();
    }while(!dropped);


    serial("x");
    do
    {
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 20);

    serial("r");
    usleep(500000);
    serial("r");
    usleep(500000);
    ResetMouse();

    serial("w");
    do
    {
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 100);
    serial("s");

    ResetMouse();

    do{
        picked = pickObject();
        printf("%d\n", picked);
    }while(!picked);    
    printf("Picked Up\n");

    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    ResetMouse();
    
    printf("Drop Started\n");
    do
    {
        dropped = dropObject();
    }while(!dropped);


    ResetMouse();
    do
    {
        serial("x");
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 20);

    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    ResetMouse();

    serial("w");
    do
    {
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 100);
    serial("s");

    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    ResetMouse();

    serial("w");
    do
    {
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 100);
    serial("s");

    ResetMouse();

    do{
        picked = pickObject();
        printf("%d\n", picked);
    }while(!picked);    
    printf("Picked Up\n");
    
    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    serial("t");
    usleep(500000);
    ResetMouse();

    serial("w");
    do
    {
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 120);
    serial("s");

    serial("r");
    usleep(500000);
    serial("r");
    usleep(500000);
    ResetMouse();

    printf("Drop Started\n");
    do
    {
        dropped = dropObject();
    }while(!dropped);


    int count = 0;
    while(1)
    {   
        searchPick();
        searchDrop();
        count++;
        printf("Object Count : %d\n", count);
        if(count == 2)
        {
            return 0;
        }
    }

    return 0;
}

/*****************************************************************************************************
                                            Reach Line
******************************************************************************************************/
void reachLine()
{   
    printf("Go to Line\n");

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
        usleep(500000);
    serial("r");
        usleep(500000);
    serial("r");
        usleep(500000);
    serial("r");
        usleep(500000);
        usleep(500000);
        usleep(500000);
}

/*****************************************************************************************************
                                            Get to New Position
******************************************************************************************************/
void newPosition()
{
    bool free;

    printf("New Position\n");

    do
    {
        serial("t");
        usleep(500000);
        free = getObject();
    }while(!free);

    reachLine();
}

/*****************************************************************************************************
                                            Search for picking object
******************************************************************************************************/
void searchPick()
{
    serial("c.100.");
    serial("v.150.");
    usleep(500000);

    bool picked = false;
    int count = 0;
    
    printf("Picking Object\n");

    do
    {
        picked = pickObject();
        printf("Turning Right\n");
        serial("t");
        usleep(500000);
        usleep(500000);
        
        count++;

        if(count > 10)
        {
            newPosition();
            return searchPick();
        }
    }while(!picked);
    printf("Object Picked\n");
    return;
}

/*****************************************************************************************************
                                            Search for dropping object
******************************************************************************************************/
void searchDrop()
{
    int dis;
    
    serial("v.170.");
    serial("c.100.");
    usleep(500000);

    bool dropped = false;        
    int count = 0;
 
    printf("Dropping Object\n");
    
    do
    {
        dropped = dropObject();
        printf("Turning Right\n");
        
        serial("t");
        usleep(500000);
        usleep(500000);

        count++;

        if(count > 10)
        {
            newPosition();
            return searchDrop();
        }
    }while(!dropped); 

    printf("Object Dropped\n");
    
    ResetMouse();
    do
    {
        serial("x");
        dis = mouse().y;
        printf("%d\n", abs(dis));
    }while(abs(dis) < 40);
    serial("s");

    ResetMouse();

    return;
}

/*****************************************************************************************************
                                            Pick Object
Input : None
Output: Boolean indicating if object has been picked

Searches for the object within the Field Of View (FOV), aligns the robot to the centre of the 
object, moves the robot close enough and initiates pickup sequence
******************************************************************************************************/
bool pickObject()
{
    printf("Function : Pick object\n");

    //Sets the camera viewing angle to 150 degrees ensuring view of the object for 300 cm
    serial("v.150.");

    /****************************************************************
    Variable Declarations
    ****************************************************************/
    Mat imageHSV, imageRGB, red, green, yellow;
    vector<vector<Point> > contours, objectContours;

    int largest_area = 0;
    int largest_contour_index=0;

    /****************************************************************
     Acquire Image and Mask for various colors
    ****************************************************************/
    imageHSV = getImage();

    inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
    inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,160),green);

    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
    
    /****************************************************************
    Identify the object on the field combine all three colors of 
    the target objects avoid green (dark blue looks green with 
    shadow on it). Only the largest countour is process (technically
    the robot can only pick one) maening the closet object will be 
    picked
    ****************************************************************/
    Mat combined;
    bitwise_or(red, yellow, combined);
    //bitwise_or(combined, green, combined);
    
    { 
        //Find the largest contour inside the image  
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

        //If there is no countour detected return a false indicating no object picked
        if(largest_contour_index != -1)
        {
            //Find an approximate rectangle for the object
            Rect object = boundingRect(objectContours[largest_contour_index]);
            
            //Find the x,y coordinate of the obstacle from the camera. 
            //The width is used so the object doesnt have to be fully inside the frame vertically
            Point object = destinationLocation(5,object);
            printf("Object : %d %d\n", object.x, object.y);

            //The minimum and maximum values for centralization for different distance range
            int min, max;
            if(object.y > 45){
                min = -2;
                max = 2;
            }else{
                min = -1;
                max = 1;
            }
            
            //Check if object is within range specified by min and max else adjust
            if(object.x > min & object.x < max){
                int dis;

                //If object is within the distance range of the IR Sensor Initiaite Pick up
                if(object.y < 45){
                    //Reset the gripper and wait for completed signal
                    printf("Initializing Gripper\n");
                    serial("i");
                    serialR();
                    printf("Gripper Initialized\n");
                    
                    //Initiate the pick command and wait for signal indicating pick
                    printf("Picking Object\n");
                    serial("p");
                    serialR();
                    printf("Object Picked\n");

                    return true;
                }

                //If object not within range move to a distance 35 cm away from the object
                serial("w");
                do
                {
                    dis = mouse().y;
                    printf("Dest:%d %d\n", object.y, dis);
                }while(dis < abs(object.y) - 35);
                serial("s");
                ResetMouse();

                //Lower the camera for the next iteration since the object is already close
                serial("v.170");
                return pickObject();
            }
            else{
                //Move the robot based on current position and call the function recursively
                if(object.x <= min){
                    int steps = object.x / 5;
                    
                    for(int i = 0; i <= abs(steps); i++)
                    {
                            turnL();   
                    }
                    usleep(5000);
                }
                else if(object.x >= max){
                    int steps = collection.x / 5;

                    for(int i = 0; i <= abs(steps); i++)
                    {
                            turnR();   
                    }
                    usleep(5000);
                }
                return pickObject();   
            }
        }

    } 

    #if 0
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/Yellow.jpg", yellow);
    imwrite("/home/pi/Red.jpg", red);
    imwrite("/home/pi/Green.jpg", green);
    imwrite("/home/pi/Combined.jpg", combined);
    #endif

    //Release all the image holders to avoid memory leak
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
Input : None
Output: Boolean indicating if object dropped

Searches for the destination within the Field Of View (FOV), aligns the robot to the centre of the 
object and drops the object on the collection poin
******************************************************************************************************/
bool dropObject()
{
    printf("Function : Drop object\n");

    //Sets the camera viewing angle to 150 degrees well within the height of the destination
    serial("v.150.");

    /****************************************************************
    Variable Declarations
    ****************************************************************/
    Mat imageHSV, imageRGB, white, blue;
    Mat contourOutput;
    vector<vector<Point> > contours, objectContours;
    vector<Point> fieldContour;

    int largest_area = 0;
    int largest_contour_index=0;

    /****************************************************************
     Acquire Image and Mask for various colors
    ****************************************************************/
    imageHSV = getImage();
   
    inRange(imageHSV, Scalar(15,0,165), Scalar(90,25,255),white);
    inRange(imageHSV, Scalar(90,60,180), Scalar(105,150,255),blue);
    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);

    /****************************************************************
    Detect the game field based on the largest contour in the blue
    mask from the image
    ****************************************************************/
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
    
    //If there is no blue field detected don't set fieldContour. Quits without throwing error
    if(largest_contour_index != -1)
        fieldContour = contours[largest_contour_index];

    /****************************************************************
    Identify the destination within the field and centralize the 
    object by adjusting the robot when centralized move within a 
    distance of 50 cm and initiate drop sequence
    ****************************************************************/
    {
        //Find the contours inside the image 
        objectContours.clear();
        findContours(white.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
       
        //Check if detected object is within the field
        //TODO : Time consuming due to large numbers of contours. Filter contours based on area before checking for points inside the field 
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

        //Improved faster algorithm 
        //TODO : To be tested
        /*for(int i = 0; i < contours.size(); i++)
        {
            for(int j = 0; j < contours[i].size(); j++)
            {
                //Test if the area is less than 4000 and distance of the object from the field and if within -5 units consider as inside the field
                if(contourArea( contours[i], false) > 4000.0)
                {
                    double test = pointPolygonTest(fieldContour, contours[i][j], true);  
                    if(test > -5)
                    {
                        objectContours.push_back(contours[i]);
                        break;
                    }
                }
            }
        }*/ 
        
        //Find the largest among the object contours
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

        //If there is a contour process else return false indicating destination not within view
        if(largest_contour_index != -1)
        {
            //Find an approximate rectangle for the object
            Rect object = boundingRect(objectContours[largest_contour_index]);
            
            //Find the x,y coordinate of the destination from the camera. The width of the destination is 30cm    
            Point collection = destinationLocation(30,object);
            printf("Collection Point : %d %d\n", collection.x, collection.y);

            //The minimum and maximum values for centralization for different distance range
            int min, max;
            if(collection.y > 60){
                min = -5;
                max = 5;
            }else{
                min = -2;
                max = 2;
            }

            //Check if object is within range specified by min and max else adjust
            if(collection.x > min  & collection.x < max){
                int dis;
                
                //Check if there is enough clearence to reach the destination 
                if(!getObject())
                {
                    printf("No Clearence\n");
                    return false;    
                }

                //Move the robot to 50cm away from the destination point
                serial("w");
                do
                {
                    dis = mouse().y;
                    printf("Dest:%d %d\n", collection.y, dis);
                }while(dis < abs(collection.y) - 50);
                
                //Initiate the drop command and wait for signal indicating drop
                printf("Dropping Object\n");
                serial("l");
                serialR();
                printf("Object Dropped\n");
                
                //Reset the gripper and wait for completed signal
                printf("Initializing Gripper\n");
                serial("i");
                serialR();
                printf("Gripper Initialized\n");
                
                ResetMouse();

                return true;
            }
            else{
                //Move the robot based on current position and call the function recursively
                if(collection.x <= min){
                    int steps = collection.x / 5;
                    
                    for(int i = 0; i <= abs(steps); i++)
                    {
                            turnL();   
                    }
                    usleep(5000);
                }
                else if(collection.x >= max){
                    int steps = collection.x / 5;

                    for(int i = 0; i <= abs(steps); i++)
                    {
                            turnR();   
                    }
                    usleep(5000);
                }
                return dropObject();   
            }
        }
    } 


    #if 0
    imwrite("/home/pi/RGB.jpg", imageRGB);
    imwrite("/home/pi/HSV.jpg", imageHSV);
    imwrite("/home/pi/White.jpg", white);
    imwrite("/home/pi/Blue.jpg", blue);
    #endif

    //Release all the image holders to avoid memory leak
    imageRGB.release();
    imageHSV.release();
    white.release();
    blue.release();
    contourOutput.release();

    return false;
}

/*****************************************************************************************************
                                            Get Object
Input : None
Output: Boolean indicating whether the path directly in front of robot is free of Obstacles

Searches for obstacles within the Field Of View (FOV) and checks if a straight movement would result 
in collision
******************************************************************************************************/
bool getObject()
{
    /****************************************************************
    Variable Declarations
    ****************************************************************/
    Mat imageHSV, imageRGB;
    Mat white, blue, black, gray;
    Mat contourOutput;
    //Mat yellow, red, green;

    vector<vector<Point> > contours, objectContours;
    vector<Point> fieldContour;

    int largest_area, largest_contour_index=0;

    /****************************************************************
     Acquire Image and Mask for various colors
    ****************************************************************/
    imageHSV = getImage();
    
    inRange(imageHSV, Scalar(15,0,165), Scalar(90,25,255),white);
    inRange(imageHSV, Scalar(90,60,180), Scalar(105,150,255),blue);
    inRange(imageHSV, Scalar(0,0,0), Scalar(180,190,90),black);
    inRange(imageHSV, Scalar(80,20,150), Scalar(120,35,255),gray);
    //inRange(imageHSV, Scalar(150,150,90), Scalar(180,240,255),red);
    //inRange(imageHSV, Scalar(25,230,90), Scalar(40,255,255),yellow);
    //inRange(imageHSV, Scalar(60,140,50), Scalar(100,255,160),green);

    cvtColor(imageHSV,imageRGB,CV_HSV2BGR);
    
    /****************************************************************
    Color Calibration
    ****************************************************************/
    #if 0
    namedWindow( "RGB", CV_WINDOW_NORMAL );
    namedWindow( "HSV", CV_WINDOW_NORMAL );
    namedWindow( "White", CV_WINDOW_NORMAL );
    namedWindow( "Blue", CV_WINDOW_NORMAL );
    namedWindow( "Black", CV_WINDOW_NORMAL );
    namedWindow( "Gray", CV_WINDOW_NORMAL );
    namedWindow( "Red", CV_WINDOW_NORMAL );
    namedWindow( "Yellow", CV_WINDOW_NORMAL );
    namedWindow( "Green", CV_WINDOW_NORMAL );

    imshow("HSV", imageHSV);
    imshow("RGB", imageRGB);
    imshow("White", white);
    imshow("Blue", blue);
    imshow("Black", black);
    imshow("White", white);
    imshow("White", gray);
    imshow("Red", red);
    imshow("Green", green);
    imshow("Yellow", yellow);

    setMouseCallback("RGB", mouseEvent, &imageHSV);
    waitKey(0);
    #endif

    /****************************************************************
    Detect the game field based on the largest contour in the blue
    mask from the image
    ****************************************************************/
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
    
    //If there is no blue field detected don't set fieldContour. Quits without throwing error
    if(largest_contour_index != -1)
        fieldContour = contours[largest_contour_index];


    /****************************************************************
    Identify the obstacles on the field and check if they are in the
    path of the robot (20 cm to either the left or the right)
    ****************************************************************/
    #if 1
    {
        //Combine both the black and gray images using bitwise-or operation
        Mat combined;
        bitwise_or(black, gray, combined);

        //Multiple obstacles can be found in the same view hence the process to be looped until empty image
        while(1)
        {
            //Find the contours inside the image 
            objectContours.clear();
            findContours( combined.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
        
            //Check if detected object is within the field
            //TODO : Time consuming due to large numbers of contours. Filter contours based on area before checking for points inside the field
            for(int i = 0; i < contours.size(); i++)
            {
                for(int j = 0; j < contours[i].size(); j++)
                {
                    //Test the distance of the object from the field and if within -5 units and area less than 4000 consider as inside the field
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

            //Improved faster algorithm 
            //TODO : To be tested
            /*for(int i = 0; i < contours.size(); i++)
            {
                for(int j = 0; j < contours[i].size(); j++)
                {
                    //Test if the area is less than 4000 and distance of the object from the field and if within -5 units consider as inside the field
                    if(contourArea( contours[i], false) > 4000.0)
                    {
                        double test = pointPolygonTest(fieldContour, contours[i][j], true);  
                        if(test > -5)
                        {
                                objectContours.push_back(contours[i]);
                                break;
                        }
                    }
                    
                }
            }*/

            //Find the largest among the object contours
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
            
            //If there is a contour process else all the objects have been processed already
            if(largest_contour_index != -1)
            {
                //Remove the object to be processed from the image for next iteration
                drawContours(combined, objectContours, largest_contour_index, Scalar(0,0,0), -1, 8);

                //Find an approximate rectangle for the object
                Rect object = boundingRect(objectContours[largest_contour_index]);
                
                //Find the x,y coordinate of the obstacle from the camera. The height of the obstacles is set to 38cm
                Point obstacle = obstacleLocation(38,object);
                printf("Obstacle : %d %d\n", obstacle.x, obstacle.y);

                //If obstacle doesn't have a clearence of 20cm for the object return false
                if(obstacle.x > -20 & obstacle.x < 20)
                {
                    return false;
                }
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
    imwrite("/home/pi/Blue.jpg", blue);
    imwrite("/home/pi/Black.jpg", black);
    imwrite("/home/pi/Gray.jpg", gray);
    #endif

    //Release all the image holders to avoid memory leak
    imageHSV.release();
    imageRGB.release();
    blue.release();
    black.release();
    gray.release();
    contourOutput.release();

    return true;
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

/*void moveTo(int x, int y)
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
*/

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