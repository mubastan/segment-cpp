/***************************************************************
 * Name:      SRMSeg.h
 * Purpose:   Defines SRM (Statistical Region Merging) segmentation class
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   27.05.2009
 * update:    14.01.2011 (for OpenCV 2.2 Mat)
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#ifndef SRMSEG__H__
#define SRMSEG__H__

#include "opencv2/core/core.hpp"

#include "DisjointSet.h"

using namespace cv;

typedef struct
{
 int reg1, reg2, delta;
} RegionPair;

class SRMSeg
{
    public:
        SRMSeg(int w, int h);
        ~SRMSeg();

        void reallocate(int w, int h);
        void allocate(int w, int h);
        void deallocate();

        void initializeMeans(Mat& image);

        void segment(Mat& image, float Q = 40.0f, float minsize = 100.0f);
        void segmentGraph(RegionPair* pairs, int numEdges);
        void mergeSmall(RegionPair* pairs, int numEdges, int minsize);

        int buildGraph4( Mat& image );
        inline int distance(Vec3b& pix1, Vec3b& pix2);

        //uchar labels (upto 255 components), may overfow!!
        void getLabels( Mat& labels );
        // integer labels
        void getLabelsInt(Mat& labels);

        /// draw the segment boundaries with the given color
        void drawSegmentBoundaries( Mat& dst, Scalar bcolor = Scalar(0,255,222) );

        /// Number of components in the current segmentation
        int getNumComps() const { return ( dsf != NULL ) ? dsf->numSets() : -1; }

    protected:

        /// current image size
        int width;
        int height;

        /// determines the coarseness/fineness of segmentation
        /// larger value more (smaller) regions
        float Q;

        float minsize;

        /// number of edges in the graph
        int numEdges;

        /// mean values for each channels, e.g., Red, Green, Blue
        float* mean1;
        float* mean2;
        float* mean3;

        /// region pairs, edges..
        RegionPair* pairs;

        /// disjoint set forest
        DisjointSet* dsf;
};

#endif
