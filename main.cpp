/***************************************************************
 * Name:     main.cpp
 * Purpose:   Driver for image segmentation algorithms SRM and EGBS
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   09.07.2010
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "GreedyGraphSeg.h"
#include "SRMSeg.h"

using namespace std;

int main(int argc, char** argv)
{
    Mat img = imread("ugurbocegi.jpg");

    if(img.empty()){ cout << "Could not read the image" << endl; return 0;}

    //medianBlur(img, img, 3);
    medianBlur(img, img, 3);

    //namedWindow("image", CV_WINDOW_FREERATIO);
    namedWindow("image", CV_WINDOW_KEEPRATIO);
    imshow("image", img);

    /*
    GreedyGraphSeg gseg(img.cols, img.rows, 100, 10);
    gseg.segmentImageColor(img);
    gseg.drawSegmentBoundaries(img, Scalar(0,255,222));
    cout << "Done! Number of components: " << gseg.getNumComps() << endl;
    //Mat labels;
    //gseg.getLabels(labels);
    */


    SRMSeg srm(img.cols, img.rows);
    srm.segment(img, 45, 10);
    srm.drawSegmentBoundaries(img);
    cout << "Done! Number of components: " << srm.getNumComps() << endl;
    Mat labels;
    srm.getLabels(labels);


    namedWindow("Segmentation", CV_WINDOW_FREERATIO);
    imshow("Segmentation", img);
    waitKey();

    //imwrite("seg2400.png", img);

    //imwrite("labels.png", labels);
    //namedWindow("regions", CV_WINDOW_FREERATIO);
    //imshow("regions", labels);
    //waitKey();

    //Mat mask(src.rows, src.cols, CV_8U, Scalar(1));

    return 0;
}
