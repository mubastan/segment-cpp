/***************************************************************
 * Name:      GreedyGraphSeg.cpp
 * Purpose:   Defines Efficient Graph Based Image Segmentation (Felzenswalb, et. al, IJCV 2004)
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   27.05.2009
 * update:    14.01.2011 (for OpenCV 2.2 Mat)
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#ifndef __GreedyGraphSeg__H__
#define __GreedyGraphSeg__H__

#include <vector>

#include "opencv2/core/core.hpp"

#include "DisjointSet.h"
#include "Edge.h"

using namespace cv;

class GreedyGraphSeg
{
public:

    GreedyGraphSeg( int width, int height, float threshold = 300, int minSize = 100, int connect = 4 );
    ~GreedyGraphSeg();

    // allocate memory according to the given size
	void allocate( int width, int height );
	void deallocate();

	void setParameters( int minsize = 100, float threshold = 300.0f, int connectivity = 4 );

    /// color only segmentation
    void segmentImageColor( Mat& image );

    // eliminate small regions by merging
    void postProcess( );

    /// increment amount for edge weight, when joining 2 sets,
    /// used in segmentGraph
    float edgeThresh(int size){ return threshold/size; }
    float edgeThresh2(int size){ return threshold/(size*size); }
    float edgeThresh3(int size){ return threshold/(expf(size)); }

    /// input: labeled, single band image; populated with labels [0..#components -1]
    // (upto 255 components), may overfow!!
	void getLabels( Mat& labels );
	// integer labels
    void getLabelsInt(Mat& labels);

    /// draw the segment boundaries with the given color
    void drawSegmentBoundaries( Mat& dst, Scalar bcolor = Scalar(255,0,0) );

    /// Number of components in the current segmentation
    int getNumComps() const { return ( dsf != NULL ) ? dsf->numSets() : -1; }


private:
    /// build graph. connect: connectivity, 4 or 8
    int buildGraph4( Mat& image );
    int buildGraph8( Mat& image );

    /// segment graph, make a dsf
    void segmentGraph( int numVertices, int numEdges);
	void segmentGraph2( int numVertices,  int numEdges );
	void segmentGraph3( int numVertices,  int numEdges );

    inline float distance(Vec3b& pix1, Vec3b& pix2);
    inline float distance(Vec3f& pix1, Vec3f& pix2);


private:

	int width;
	int height;

    /// used in segmentGraph() to decide if to join two sets
    float threshold;

    /// min region size (connected component) in the segmentation
    int minSize;

    /// 4 or 8 connectivity in building the graph
    int connect;

    /// number of edges in the graph
    int numEdges;

    // total frame area: width*height
    float area;

    ///
    edge* edges;

    /// disjoint set forest
    DisjointSet* dsf;

    /// thresholds in segmentGraph
    float* thresholds;

};

#endif
