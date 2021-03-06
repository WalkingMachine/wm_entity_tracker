//
// Created by philippe on 01/11/18.
//
#include "EntityTracker.h"

using namespace std;

EntityTracker::EntityTracker() : mNextID{1} {
    perceptionMutex.unlock();
}

EntityTracker::~EntityTracker() {
    mEntities.clear();
}

void EntityTracker::update(ros::Duration deltaTime) {
    perceptionMutex.lock();
    // Reduce slightly the existance probability to eliminate old entities.
    for (auto &entity : mEntities) {
        entity.probability -= decayRate();
    }

    // merge entities togeter if too similar
    for (auto &entity1 : mEntities) {
        if (entity1.probability > 0){
            for (auto &entity2 : mEntities) {
                if (entity1.ID != entity2.ID && entity1.compareWith(entity2) < mCouplingThreashold){
                    if (entity1.ID > entity2.ID){
                        entity2.mergeOnto(entity1);
                        entity1.probability = -1;
                    } else {
                        entity1.mergeOnto(entity2);
                        entity2.probability = -1;
                    }
                }
            }
        }
    }


    // Delete all old entities.
    deleteDeads();

    // Update all entities.
    for (auto &entity : mEntities) {
        entity.update(deltaTime);
    }

    vector<PerceivedEntity> entities;
    for (auto &entity : mEntities) {
        if (entity.probability > publicationTreashold())
            entities.push_back(entity);
    }

    // Write into all outputs
    for (auto output : mEntitiesOutput) {
        output->writeEntities(entities);
    }
    perceptionMutex.unlock();
}

bool entityIsDead(PerceivedEntity &entity) {
    return entity.probability < 0;
}

void EntityTracker::deleteDeads() {
    mEntities.erase(std::remove_if(mEntities.begin(), mEntities.end(), entityIsDead), mEntities.end());
}

void EntityTracker::perceiveEntity(PerceivedEntity entity, bool canCreate, PerceivedEntity::KalmanParams params) {

    // Create a list of entities to call perceiveEntities.
    vector<PerceivedEntity> entities;
    entities.push_back(entity);
    return perceiveEntities(entities, canCreate, params);
}

void
EntityTracker::perceiveEntities(std::vector<PerceivedEntity> perceivedEntities, bool canCreate, PerceivedEntity::KalmanParams params) {

    // Write into all outputs
    for (auto output : mEntitiesOutput) {
        output->writePerceptions(perceivedEntities);
    }

    perceptionMutex.lock();
    for (auto &perceived : perceivedEntities) {
        //cout << "prob= " << to_string(perceived.probability) << "\n";

        // Initialise the flag that tells if a match has been found.
        bool notFoundYet{true};

        // If the ID is already defined by the input, we try to update the corresponting entity.
        if (perceived.probability == 1 && perceived.ID != 0) {
            for (auto &entity : mEntities) {
                if (entity.ID == perceived.ID) {
                    notFoundYet = false;
                    entity.mergeOnto(perceived, params);
                    break;
                }
            }
            if (notFoundYet && canCreate) {
                notFoundYet = false;
                addEntity(perceived);
            }
        }
        if (notFoundYet) {
            // Use an outside loop to find the closest entity and an inside loop to validate that the match is the best around.
            // The inside loop will stop if there is already a better match.
            float minDiff{maximumDifference()};
            PerceivedEntity *closest{nullptr};
            for (auto &entity : mEntities) {
                auto diff{entity.compareWith(perceived)};
                if (diff < minDiff) {
                    bool closest2{true};
                    for (auto &perceived2 : perceivedEntities) {
                        if (entity.compareWith(perceived2) < diff) {
                            closest2 = false;
                            break;
                        }
                    }
                    if (closest2) {
                        minDiff = diff;
                        closest = &entity;
                    }
                }
            }
            if (closest) {
                // If the match is found, we update the entity.
                closest->mergeOnto(perceived, params);
            } else if (canCreate) {
                // If not, we create the new entity and add it to the list.
                // TODO: Maybe should not always result in creation. (maybe timer based IDK)
                addEntity(perceived);
            }

        }
    }
    perceptionMutex.unlock();
}


void EntityTracker::addEntity(PerceivedEntity &newEntity) {

    // Initialise the ID if needed.
    if (newEntity.ID == 0) newEntity.ID = mNextID++;

    newEntity.probability *= 0.5;  // TODO ajouter paramètre
    mEntities.push_back(newEntity);
    return;
}

void EntityTracker::addOutput(EntityOutput &output) {
    mEntitiesOutput.push_back(&output);
}

void EntityTracker::clearOutputs() {
    mEntitiesOutput.clear();
}
