#include <iostream>
#include <stdlib.h>
#include <termios.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <ros/ros.h>
#include <sensor_msgs/PointCloud.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Pose2D.h>
#include <tf/transform_broadcaster.h>


// Helper function
double distanceBetweenTwoPoints(double x1, double y1, double x2, double y2){
	return sqrt(pow(x2 - x1, 2.0) + pow(y2 - y1, 2.0));
}

class Pioneer
{
    public:
        Pioneer(ros::NodeHandle& nh);
        virtual ~Pioneer();
    protected:
        void sonar_calback(const sensor_msgs::PointCloudConstPtr& cloud_msg);
        void odometry_calback(const nav_msgs::Odometry::ConstPtr& msg);
        ros::Subscriber odom_pose_sub;
        ros::Subscriber sonar_sub;
        ros::Subscriber bumper_sub;
        ros::Publisher vel_pub;
        geometry_msgs::Pose2D robot_pose;
};


Pioneer::Pioneer(ros::NodeHandle& nh){
    vel_pub = nh.advertise<geometry_msgs::Twist>("RosAria/cmd_vel", 1);

    // Make the robot stop (robot perhaps has a speed already)
    geometry_msgs::Twist msg;

    for (int i=0; i<3; i++){
        msg.linear.x = 0;
        msg.linear.y = 0;
        msg.linear.z = 0;
        msg.angular.x = 0;
        msg.angular.y = 0;
        msg.angular.z = 0;
        vel_pub.publish(msg);
    }

    sonar_sub = nh.subscribe("/RosAria/sonar", 1000, &Pioneer::sonar_calback, this);
    odom_pose_sub = nh.subscribe("/RosAria/pose", 1000, &Pioneer::odometry_calback, this);
}  

Pioneer::~Pioneer()
{
  ROS_INFO_STREAM("Quitting... \n");
}

void Pioneer::sonar_calback(const sensor_msgs::PointCloudConstPtr& cloud_msg) {
    float linear_vel = 0.2;
    float ang_vel = 0.2;
    // mean steering angle
    float mean_ang = 1.5708;

    float x = 0;
    float y = 0;
    for (int i= 1; i < cloud_msg->points.size() - 9; i++) {
        x += cloud_msg->points[i].x;
        y += cloud_msg->points[i].y;
        
    }
    float dist = sqrt(pow(x, 2.0) + pow(y, 2.0));
    float angle = atan2(x, y);
    // ROS_INFO_STREAM("Distance: " << dist << " Angle: " << angle);

    geometry_msgs::Twist msg;

    if (dist < 6) {
        msg.linear.x = 0;
        msg.angular.z = (mean_ang + ang_vel) - angle;
    } else {
        msg.angular.z = mean_ang - angle;
        msg.linear.x = linear_vel;
    }

    vel_pub.publish(msg);
}


void Pioneer::odometry_calback(const nav_msgs::Odometry::ConstPtr& msg){
    this->robot_pose.x = msg->pose.pose.position.x;
    this->robot_pose.y = msg->pose.pose.position.y;
}

void exitHandler(int sig)
{
    std::cout << "Node received " << sig << " and exited." << std::endl;
    ros::shutdown();
}


int main(int argc, char **argv)
{
	// Initialize the ROS system and become a node.
	ros::init(argc, argv, "RosAria_interface");
    ros::NodeHandle nh;
	Pioneer* pioneer = new Pioneer(nh);

	ros::Rate loop_rate(100);
	
	while (ros::ok()) {
		ros::spinOnce();
        loop_rate.sleep();
	}
}