#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("Moving the bot");

    // Request velocity change to lin_x, ang_z
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the drive_bot service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int ball_min_index = img.step; // horizontal; since it's the significant axis
    int ball_max_index = 0;
    bool ball_found = false;

    // Loop through each pixel in the image and check if there's a bright white one
    for (int i = 1; i < img.height * img.step -1; i++) {
        if (img.data[i] == white_pixel && img.data[i -1] == white_pixel && img.data[i+1] == white_pixel) {
            ball_found = true;
            if (i % img.height > ball_max_index){
                ball_max_index = i % img.height;
            }
            if (i % img.height < ball_min_index){
                ball_min_index = i % img.height;
            }
        }
    }
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    if(ball_found){
        float ball_center =  (ball_max_index + ball_min_index)/2.0;
        float ball_width = ball_max_index - ball_min_index;
        drive_robot(0.25*(1.0 - ball_width/img.step) , ( (0.2*(img.step))- ball_center)/(0.2*img.step));
        /*
        if (ball_center- 0.5*img.step > 0){
            // turn right
            drive_robot(0.1,0.5);
        } else if (ball_center - 0.5*img.step < 0) {
            // turn left
            drive_robot(0.1,-0.5);

        } else {
            // move forward
            drive_robot(0.3,0);
        }*/

    } else {
        // Request a stop when there's no white ball seen by the camera
        drive_robot(0,0);
    }
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}