//
// Created by philippe on 01/11/18.
//
#include "EntityTracker.h"

#define drawCross( center, color, d ) \
line( img, Point( center.x - d, center.y - d ), Point( center.x + d, center.y + d ), color, 2, CV_AA, 0); \
line( img, Point( center.x + d, center.y - d ), Point( center.x - d, center.y + d ), color, 2, CV_AA, 0 )

using namespace std;
using namespace cv;
using namespace sara_msgs;

EntityTracker::EntityTracker(ros::Duration deleteDelay) :
        mDeleteDelay{deleteDelay}{}

EntityTracker::~EntityTracker() {
    mEntities.clear();
}

void EntityTracker::update(ros::Duration deltaTime) {

    // Delete all old entities.
    deleteOld();

    // Update all entities.
    for (auto &entity : mEntities){
        entity.update(deltaTime);
    }
}

void EntityTracker::deleteOld(){
    // Create a list of entities to delete.
    vector<int> toDelete;
    int i{0};
    for (auto &entity : mEntities){
        if (entity.lastUpdateTime < ros::Time::now()-deleteDelay()){
            toDelete.push_back(i);
        }
        ++i;
    }

    // Delete all marked entities.
    for (auto &del : toDelete){
        mEntities.erase(mEntities.begin()+del);
    }
}

void EntityTracker::publishMarkers() const {
    // TODO
}

void EntityTracker::publishOnTopic() const {
    // TODO
}

bool EntityTracker::perceiveEntity(Entity entity){

    // Create a list of entities to call perceiveEntities.
    vector<Entity> entities;
    entities.push_back(entity);
    return perceiveEntities(entities);
}

bool EntityTracker::perceiveEntities(std::vector<Entity> entities){

    // Over complicated and sluggish way to match detections and entities together.
    for (auto &perceived : entities){

        // Use an outside loop to find the closest entity and an inside loop to validate that the match is the best around.
        // The inside loop will stop if there is already a better match.
        float minDiff{32000.f};
        PerceivedEntity *closest{nullptr};
        for (auto &entity : mEntities){
            auto diff{entity.compareWith(perceived)};
            if (diff < minDiff){
                float minDiff2{diff};
                Entity *closest2{nullptr};
                for (auto &perceived2 : entities){
                    auto diff2{entity.compareWith(perceived2)};
                    if (diff2 < minDiff2){
                        closest2 = &perceived;
                        minDiff2 = diff2;
                    }
                }
                if (closest2 != &perceived){
                    minDiff = diff;
                    closest = &entity;
                }
            }
        }
        if (closest){
            // If the match is found, we update the entity.
            closest->lastUpdateTime = ros::Time::now();
            closest->mergeOnto(perceived);
        } else {
            // If not, we create the new entity and add it to the list.
            // TODO: Maybe should not always result in creation. (maybe timer based IDK)
            PerceivedEntity newEntity(
                    perceived.position.x,
                    perceived.position.y,
                    perceived.position.z,
                    perceived.name);
            newEntity.lastUpdateTime = ros::Time::now();

            mEntities.push_back(newEntity);
        }
    }

    return true;
}


void EntityTracker::opencvDraw(Mat img) const{
    for (auto& entity : mEntities) {
        Point myEntity(entity.position.x, entity.position.y);
        drawCross( myEntity, Scalar(255,255,255), 5 );
    }
    putText(img, "entities = " + to_string(mEntities.size()), Point(20, 20), FONT_HERSHEY_COMPLEX, 1, 255);
}


ros::Duration EntityTracker::deleteDelay() const{
    return mDeleteDelay;
}

ros::Duration EntityTracker::setDeleteDelay(ros::Duration value){
    mDeleteDelay = value;
    return mDeleteDelay;
}