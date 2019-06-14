//
// Created by philippe on 08/11/18.
//

#ifndef PROJECT_PeoplePosesInput_H
#define PROJECT_PeoplePosesInput_H

#include "EntityInput.h"
#include "ros/ros.h"
#include <vector>
#include "sara_msgs/Poses.h"

class PosesInput : public EntityInput {
    ros::NodeHandle mNodeHandle;
    ros::Subscriber mPosesSub;

    void posesCallback(sara_msgs::Poses poses);

    ros::AsyncSpinner mSpinner;
public:
    PosesInput(EntityTracker &tracker, ros::NodeHandle &nh, std::string topic);

    ~PosesInput();
};


#endif //PROJECT_PeoplePosesInput_H
