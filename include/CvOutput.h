//
// Created by philippe on 08/11/18.
//

#ifndef PROJECT_CVOUTPUT_H
#define PROJECT_CVOUTPUT_H

#include "EntityOutput.h"
#include <vector>
#include "PerceivedEntity.h"
#include "opencv2/highgui/highgui.hpp"

#define SCREENSIZEX 800
#define SCREENSIZEY 600

class CvOutput : public EntityOutput {

    cv::Mat img;
    float mMinX;
    float mMinY;
    float mScalingX;
    float mScalingY;

    void adaptScreen(const PerceivedEntity &entity);

public:
    CvOutput(float minX = -10, float minY = -10, float maxX = 10, float maxY = 10);

    ~CvOutput() override;

    void writeEntities(const std::vector<PerceivedEntity> &entities) override;

    void writePerceptions(const std::vector<PerceivedEntity> &entities) override;

};


#endif //PROJECT_CVOUTPUT_H
