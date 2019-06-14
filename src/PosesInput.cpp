//
// Created by philippe on 08/11/18.
//

#include "PosesInput.h"
#include <iostream>

using namespace std;
using namespace sara_msgs;

PosesInput::PosesInput(EntityTracker &tracker, ros::NodeHandle &nh, const string topic)
        : EntityInput(tracker),
          mNodeHandle(nh),
          mSpinner(1),
          mPosesSub() {
    mPosesSub = nh.subscribe(topic, 10, &PosesInput::posesCallback, this);
    mSpinner.start();
}

PosesInput::~PosesInput() {
    mSpinner.stop();
}

void PosesInput::posesCallback(sara_msgs::Poses poses) {
    vector<PerceivedEntity> entities;

    for (auto pose : poses.poses) {

        PerceivedEntity en;

        // Calculate mean position
        for (auto part : pose.parts){
            en.position.x += part.position.x;
            en.position.y += part.position.y;
        }
        en.position.x /= pose.parts.size();
        en.position.y /= pose.parts.size();

        
        en.face.boundingBox.Center.z = 1.5;
        en.probability = 1.0;
        en.name = "person";
        en.lastUpdateTime = poses.header.stamp;

        entities.push_back(en);
    }
    perceive(entities, false);
}