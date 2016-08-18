/***************************************************************
 * Name:      GreedyGraphSeg.cpp
 * Purpose:   Defines Efficient Graph Based Image Segmentation (Felzenswalb, et. al, IJCV 2004)
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   27.05.2009
 * update:    14.01.2011 (for OpenCV 2.2 Mat)
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>

#include "GreedyGraphSeg.h"

#define THRESHOLD(size, c) (c/size)

inline float square( double val){return val*val;}

using namespace std;

/**
 * Constructor
 */
GreedyGraphSeg :: GreedyGraphSeg( int width, int height, float threshold, int minSize, int connect )
{
	// this must be called first, since parameters are used in allocate
    this->setParameters( minSize, threshold, connect );
	this->allocate( width, height );
}

void GreedyGraphSeg :: allocate( int width, int height )
{
	if( width < 1 || height < 1 )
        throw "GreedyGraphSeg :: allocate - Illegal width-height for image!";

	this->width = width;
	this->height = height;

	// will keep the graph edges, size is more than required
    this -> edges = new edge[ width * height * ( this ->connect / 2 ) ];

    // DSF, numVertices: width*height (one node for each pixel)
    this -> dsf = new DisjointSet( width*height );

    // thresholds array, numVertices: width*height
    this -> thresholds = new float[ width*height ];

    if( !this -> edges || !this -> dsf || !this -> thresholds )
        throw "Memory allocation failed! GreedyGraphSeg::GreedyGraphSeg()";

    this->area = width*height;

}

void GreedyGraphSeg :: setParameters( int minsize, float threshold, int connectivity )
{
	if ( minsize < 1 )
		throw "GreedyGraphSeg :: setParameters - Illegal minsize for segmentation!";

	if( threshold < 1.0f )
		throw "GreedyGraphSeg :: setParameters - Illegal threshold for segmentation!";

	if( connectivity != 4 || connectivity != 8 )
		connectivity = 4;

	this->minSize = minsize;
	this->threshold = threshold;
	this->connect = connectivity;
}


/**
 * destructor
 */
GreedyGraphSeg :: ~GreedyGraphSeg()
{
    this->deallocate();
}

void GreedyGraphSeg :: deallocate()
{
	if( edges )
        delete[] edges;
    edges = NULL;

    if( dsf)
        delete dsf;
    dsf = NULL;

    if( thresholds )
        delete[] thresholds;
    thresholds = NULL;
}

/**
 * segment image based on color only
 */
void GreedyGraphSeg :: segmentImageColor( Mat& image )
{
    if(image.empty())
        return;

	if( image.cols != this->width || image.rows != this->height )
	{
		this->deallocate();
		this->allocate( image.cols, image.rows );
	}

    int numEdges;
    if( this->connect == 4 )
        numEdges = buildGraph4( image );
    else
        numEdges = buildGraph8( image );

    this->numEdges = numEdges;


    // segment the graph and create a DSF
    dsf->reset();
    //this->segmentGraph( (image.cols) * (image.rows), numEdges );
    //this->segmentGraph3( (image.cols) * (image.rows), numEdges );

    // eliminate small components
    this->postProcess();

}

/**
 * Segment a graph, greedy cut
 *
 * Returns a disjoint-set forest representing the segmentation.
 *
 * numVertices: number of vertices in graph.
 * numEdges: number of edges in graph
 *
 **/
void  GreedyGraphSeg :: segmentGraph( int numVertices,  int numEdges )
{

    int i;

    // sort edges by weight
    std::sort(edges, edges + numEdges );

    // initialize threshols
    for (i = 0; i < numVertices; i++)
	    thresholds[i] = THRESHOLD(1, this->threshold );


    // for each edge, in non-decreasing weight order...
    edge* pedge = 0;
    for (i = 0; i < numEdges; i++) {

	    pedge = &edges[i];

		// components conected by this edge
		int a = dsf -> find( pedge->a );
		int b = dsf -> find( pedge->b );
		if ( a != b ) {
		    if ( ( pedge->w <= thresholds[a] ) && ( pedge->w <= thresholds[b] ) ) {
			    dsf->join(a, b);
				a = dsf->find(a);
				thresholds[a] = pedge->w + THRESHOLD( dsf->setSize(a), this->threshold );
		    }
	    }
    }

}

/**
 * Segment a graph, greedy cut
 *
 * Returns a disjoint-set forest representing the segmentation.
 *
 * numVertices: number of vertices in graph.
 * numEdges: number of edges in graph
 *
 * does not initialize the thresholds
 **/
void  GreedyGraphSeg :: segmentGraph2( int numVertices,  int numEdges ) {

    int i;

    // sort edges by weight
    std::sort(edges, edges + numEdges );

    // initialize threshols
    //for (i = 0; i < numVertices; i++)
	//    thresholds[i] = THRESHOLD(1, this->threshold );


    // for each edge, in non-decreasing weight order...
    edge* pedge = 0;
    for (i = 0; i < numEdges; i++) {

	    pedge = &edges[i];

		// components conected by this edge
		int a = dsf -> find( pedge->a );
		int b = dsf -> find( pedge->b );
		if ( a != b ) {
		    if ( ( pedge->w <= thresholds[a] ) && ( pedge->w <= thresholds[b] ) ) {
			    dsf->join(a, b);
				a = dsf->find(a);
				thresholds[a] = pedge->w + THRESHOLD( dsf->setSize(a), this->threshold );
		    }
	    }
    }

}

void  GreedyGraphSeg :: segmentGraph3( int numVertices,  int numEdges )
{
    int i;

    // sort edges by weight
    std::sort(edges, edges + numEdges );

    // initialize threshols
    for (i = 0; i < numVertices; i++)
	    thresholds[i] = THRESHOLD(1, this->threshold );

    float TT = this->threshold;

    // for each edge, in non-decreasing weight order...
    edge* pedge = 0;
    for (i = 0; i < numEdges; i++) {

	    pedge = &edges[i];

		// components conected by this edge
		int a = dsf -> find( pedge->a );
		int b = dsf -> find( pedge->b );
		if ( a != b ) {
		    if ( ( pedge->w <= TT ) && ( pedge->w <= TT ) ) {
			    dsf->join(a, b);
				a = dsf->find(a);
				TT = pedge->w + edgeThresh( dsf->setSize(a) );

				thresholds[a] = pedge->w + THRESHOLD( dsf->setSize(a), this->threshold );
		    }
	    }
    }

    for (i = 0; i < numEdges; i++) {

	    pedge = &edges[i];

		// components conected by this edge
		int a = dsf -> find( pedge->a );
		int b = dsf -> find( pedge->b );
		if ( a != b ) {
		    if ( ( pedge->w <= thresholds[a] ) && ( pedge->w <= thresholds[b] ) )
		    {
			    dsf->join(a, b);
				a = dsf->find(a);
				//TT = pedge->w + edgeThresh( dsf->setSize(a) );

				thresholds[a] = pedge->w + THRESHOLD( dsf->setSize(a), this->threshold );
		    }
	    }
    }

}

/**
 *  eliminate small regions by merging
 */
void GreedyGraphSeg :: postProcess( ){
    int i, a, b;
    // post process small components
    for ( i = 0; i < this->numEdges; i++ ) {
        a = dsf->find( edges[i].a );
        b = dsf->find( edges[i].b );
        if ( (a != b) && ( (dsf->setSize(a) < minSize) || ( dsf->setSize(b) < minSize)))
            dsf->join(a, b);
    }

}

/**
 * Build graph, 4 connected
 */
int GreedyGraphSeg :: buildGraph4( Mat& image ){

    int width = image.cols;
    int height = image.rows;

    int numEdges = 0;
    int yw = 0;     //y * width
    int ywx = 0;    //y * width + x
    int x,y;
    for (y = 0; y < height; y++) {

        yw = y * width;
        for (x = 0; x < width; x++) {

            ywx = yw + x;
            if ( x < width - 1 ) {
                this->edges[numEdges].a = ywx;        //	y * width + x
                this->edges[numEdges].b = ywx + 1;    //  y * width + (x + 1)

	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y, x+1);
	            this->edges[numEdges].w = distance( pix1, pix2 );
	            numEdges++;
            }

            if ( y < height - 1 ) {
                this->edges[numEdges].a = ywx;            // y * width + x;
                this->edges[numEdges].b = ywx + width;    // (y+1) * width + x;
	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y+1, x);
	            this->edges[numEdges].w = distance( pix1, pix2 );
	            numEdges++;
            }

        }
    }

    return numEdges;
}


/**
 * Build graph, 8 connected
 */
int GreedyGraphSeg :: buildGraph8( Mat& image )
{
    int width = image.cols;
    int height = image.rows;

    int numEdges = 0;
    int yw = 0;     //y * width
    int ywx = 0;    //y * width + x
    int x,y;
    for (y = 0; y < height; y++) {

        yw = y * width;
        for (x = 0; x < width; x++) {

            ywx = yw + x;
            if ( x < width - 1 ) {
                this->edges[numEdges].a = ywx;        //	y * width + x
                this->edges[numEdges].b = ywx + 1;    //  y * width + (x + 1)
	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y, x+1);
	            this->edges[numEdges].w = distance( pix1, pix2 );
	            numEdges++;
            }

            if (y < height-1) {
                this->edges[numEdges].a = ywx;            // y * width + x;
                this->edges[numEdges].b = ywx + width;    // (y+1) * width + x;
	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y+1, x);
	            this->edges[numEdges].w = distance( pix1, pix2 );
	            numEdges++;
            }

            if ((x < width-1) && (y < height-1)) {
                this->edges[numEdges].a = ywx;            // y * width + x;
	            this->edges[numEdges].b = ywx + width + 1;    // (y+1) * width + (x + 1);
	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y+1, x+1);
	            this->edges[numEdges].w = distance( pix1, pix2 );
	            numEdges++;
            }

            if ((x < width-1) && (y > 0)) {
	            this->edges[numEdges].a = ywx;            // y * width + x;
	            this->edges[numEdges].b = ywx - width + 1;    // (y-1) * width + (x + 1);
	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y-1, x+1);
	            this->edges[numEdges].w = distance( pix1, pix2 );
	            numEdges++;
            }
        }
    }

    return numEdges;
}

/**
 * distance between two pixels
 */
inline float GreedyGraphSeg :: distance(Vec3b& pix1, Vec3b& pix2)
{
    return sqrt( square( pix1[0] - pix2[0] ) + square( pix1[1] - pix2[1] ) + square( pix1[2] - pix2[2] ) );
}

inline float GreedyGraphSeg :: distance(Vec3f& pix1, Vec3f& pix2)
{
    return sqrt( square( pix1[0] - pix2[0] ) + square( pix1[1] - pix2[1] ) + square( pix1[2] - pix2[2] ) );
}

/// 0 ... numComps-1
void GreedyGraphSeg :: getLabels( Mat& labels )
{

    if( !dsf )
        throw "Null pointer, dsf! GreedyGraphSeg::getLabeledImage()";

	int w = this->width;
    int h = this->height;

    if(labels.empty())
        labels.create(h, w, CV_8UC1 );

    vector <int> sids;

    bool found;
    int s;
    int yw;
    for ( int y = 0; y < h; y++ ) {
        yw = y * w;
        for ( int x = 0; x < w; x++ ) {

          int comp = dsf->find(yw + x);
          found = false;
          for ( int k = sids.size() - 1; k >= 0 ; k-- ){
              if( comp == sids[k] ){
                  found = true;
                  s = k;

                  break;
              }
          }

          if( !found )
          {
              sids.push_back( comp );
              s = (int)sids.size() - 1;
          }

          // give label to this pixel, single band image
          // labels start from 0, id of the component
          labels.at<uchar>(y,x) = (uchar)s;

        }
    }
}

/// 0 ... numComps-1
void GreedyGraphSeg :: getLabelsInt( Mat& labels )
{

    if( !dsf )
        throw "Null pointer, dsf! GreedyGraphSeg::getLabeledImage()";

	int w = this->width;
    int h = this->height;

    if(labels.empty())
        labels.create(h, w, CV_32SC1 );

    vector <int> sids;

    bool found;
    int s;
    int yw;
    for ( int y = 0; y < h; y++ ) {
        yw = y * w;
        for ( int x = 0; x < w; x++ ) {

          int comp = dsf->find(yw + x);
          found = false;
          for ( int k = sids.size() - 1; k >= 0 ; k-- ){
              if( comp == sids[k] ){
                  found = true;
                  s = k;

                  break;
              }
          }

          if( !found )
          {
              sids.push_back( comp );
              s = (int)sids.size() - 1;
          }

          // give label to this pixel, single band image
          // labels start from 0, id of the component
          labels.at<int>(y,x) = s;

        }
    }

}

/**
 * draw the boundaries of the segments with the given color on dst image
 */
void GreedyGraphSeg :: drawSegmentBoundaries( Mat& dst, Scalar bcolor )
{

    if( !dsf )
        return;

    int w = this->width;
    int h = this->height;

    if(dst.empty())
        dst.create(h, w, CV_8UC3 );


    bool flag = false;
    int cid, cidx, cidy;
    for ( int y = 0; y < h - 1 ; y++ )
    {
        for ( int x = 0; x < w - 1 ; x++ )
        {
            cid = dsf->find(y * w + x);
            cidx = dsf->find(y * w + x + 1);      // right pixel
            cidy = dsf->find( (y + 1) * w + x);   // bottom pixel

            flag = false;
            if(cid != cidx)
            {
                //dst.at<Vec3b>(y,x+1) = bcolor;
                cv::Mat roi(dst, Rect(x, y, 1, 1));
                roi = bcolor;
                //cvSet2D( dst, y, x+1, bcolor );
                flag = true;
            }

            if(cid != cidy)
            {
                //cvSet2D( dst, y+1, x, bcolor );
                //dst.at<Vec3b>(y+1,x) = bcolor;
                cv::Mat roi(dst, Rect(x, y, 1, 1));
                roi = bcolor;
                flag = true;
            }

            //if(flag)
            //    dst.at<Vec3b>(y,x) = bcolor;
                //cvSet2D( dst, y, x, bcolor );
        }
    }

}
