/***************************************************************
 * Name:      SRMSeg.cpp
 * Purpose:   Code for SRM (Statistical Region Merging) segmentation
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   27.05.2009
 * update:    14.01.2011 (for OpenCV 2.2 Mat)
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#define MIN2( A, B ) ( ( A ) < ( B ) ? ( A ) : ( B ) )
#define MAX2( A, B ) ( ( A ) < ( B ) ? ( B ) : ( A ) )
#define MIN3( A, B, C ) MIN2 ( ( A ), MIN2 ( ( B ), ( C ) ) )
#define MAX3( A, B, C ) MAX2 ( ( A ), MAX2 ( ( B ), ( C ) ) )

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "SRMSeg.h"

// compare edge weights, needed in STL - sort, edges are sorted according to weights
bool operator<( const RegionPair &a, const RegionPair &b )
{
  return a.delta < b.delta;
}

SRMSeg::SRMSeg(int w, int h)
{
    this->mean1 = 0;
    this->mean2 = 0;
    this->mean3 = 0;
    this->pairs = 0;
    this->dsf = 0;

    this->allocate(w,h);
}

SRMSeg::~SRMSeg()
{
    this->deallocate();
}

void SRMSeg::reallocate(int w, int h)
{
    if(w != this->width || h != this->height)
    {
        deallocate();
        allocate(w,h);
    }
}

void SRMSeg::allocate(int w, int h)
{
    this->width = w;
    this->height = h;

    int numpixels = w * h;

    this->numEdges = 2 * (h-1) * (w-1) + (h-1) + (w-1);
    this->pairs = ( RegionPair * )new  RegionPair [ this->numEdges ];

    this->mean1 = new float [numpixels];
    this->mean2 = new float [numpixels];
    this->mean3 = new float [numpixels];

    // DSF, numVertices: width*height (one node for each pixel)
    this -> dsf = new DisjointSet( numpixels );
}

void SRMSeg::deallocate()
{
    if(this->mean1)
        delete[] this->mean1;
    this->mean1 = 0;

    if(this->mean2)
        delete[] this->mean2;
    this->mean2 = 0;

    if(this->mean3)
        delete[] this->mean3;
    this->mean3 = 0;

    if(this->pairs)
        delete[] this->pairs;
    this->pairs = 0;

    if(this->dsf)
        delete this->dsf;
    this->dsf = 0;
}

/**
 * initialize the mean for the 3 channels with the pixel values
 */
void SRMSeg::initializeMeans(Mat& image)
{
    int width = image.cols;
    int height = image.rows;

    int x, y;
    int index = 0;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            Vec3b& pix = image.at<Vec3b>(y, x);

            this->mean1[index] = pix[0];
            this->mean2[index] = pix[1];
            this->mean3[index] = pix[2];
            index++;
        }
    }
}

void SRMSeg::segment(Mat& image, float Q, float minsize)
{
    this->reallocate(image.cols, image.rows);

	this->Q = Q;
	this->minsize = minsize;

    this->initializeMeans(image);

    this->numEdges = buildGraph4( image );

    this->dsf->reset();


    this->segmentGraph(this->pairs, this->numEdges);
    //this->segmentGraph2(image, this->pairs, this->numEdges);


    this->mergeSmall(this->pairs, this->numEdges, this->minsize);
}

/**
 * Build graph, 4 connected
 */
int SRMSeg :: buildGraph4( Mat& image )
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
            if ( x < width - 1 )
            {
                this->pairs[numEdges].reg1 = ywx;        //	y * width + x
                this->pairs[numEdges].reg2 = ywx + 1;    //  y * width + (x + 1)

	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y, x+1);
	            this->pairs[numEdges].delta = distance( pix1, pix2 );
	            numEdges++;
            }

            if ( y < height - 1 )
            {
                this->pairs[numEdges].reg1 = ywx;            // y * width + x;
                this->pairs[numEdges].reg2 = ywx + width;    // (y+1) * width + x;

	            Vec3b& pix1 = image.at<Vec3b>(y, x);
	            Vec3b& pix2 = image.at<Vec3b>(y+1, x);
	            this->pairs[numEdges].delta = distance( pix1, pix2 );
	            numEdges++;
            }

        }
    }

    return numEdges;
}

#define NUM_GRAY 256    // number of gray levels in 8-bit images

using namespace std;
void SRMSeg :: segmentGraph(RegionPair* pairs, int numEdges)
{
    // sort edges by weight
    std::sort(pairs, pairs + numEdges );

    float logdelta = 2.0 * log ( 6.0 * this->width*this->height );
    float threshfactor = ( NUM_GRAY * NUM_GRAY ) / ( 2.0 * this->Q );
    cout << "logdelta: " << logdelta << endl;
    cout << "threshfactor: " << threshfactor << endl;

    // for each edge, in non-decreasing weight order...
    RegionPair* pair = 0;
    int reg1, reg2, reg;
    int size1, size2, size;
    float threshold;
    for (int i = 0; i < numEdges; i++)
    {
        pair = &pairs[i];
        reg1 = this->dsf -> find( pair->reg1 );
        reg2 = this->dsf -> find( pair->reg2 );
        if( reg1 != reg2 )
        {
            size1 = this->dsf->setSize(reg1);
            size2 = this->dsf->setSize(reg2);
            threshold = sqrt ( threshfactor * ( ( ( MIN2 ( NUM_GRAY, size1 ) * log ( 1.0 + size1 ) + logdelta ) / (float)size1 ) +
			  ( ( MIN2 ( NUM_GRAY, size2 )* log ( 1.0 + size2 ) + logdelta ) / (float)size2) ) );

            // merge if distance is less than threshold
            if( fabs(this->mean1[reg1] - this->mean1[reg2]) < threshold
               && fabs(this->mean2[reg1] - this->mean2[reg2]) < threshold
               && fabs(this->mean3[reg1] - this->mean3[reg2]) < threshold )
            {
                dsf->join(reg1, reg2);  // merge two regions
                reg = dsf->find(reg1);  // resulting region
                size = size1 + size2;   // size of the resulting region

                // update mean values
                this->mean1[reg] = ( (size1*this->mean1[reg1]) + (size2*this->mean1[reg2]) )/size;
                this->mean2[reg] = ( (size1*this->mean2[reg1]) + (size2*this->mean2[reg2]) )/size;
                this->mean3[reg] = ( (size1*this->mean3[reg1]) + (size2*this->mean3[reg2]) )/size;
            }
        }
    }
}

/// merge small components (< minsize)
void SRMSeg :: mergeSmall(RegionPair* pairs, int numEdges, int minsize)
{
    RegionPair* pair = 0;
    int reg1, reg2;
    int size1, size2;
    for (int i = 0; i < numEdges; i++)
    {
        pair = &pairs[i];
        reg1 = this->dsf -> find( pair->reg1 );
        reg2 = this->dsf -> find( pair->reg2 );
        if( reg1 != reg2 )
        {
            size1 = this->dsf->setSize(reg1);
            size2 = this->dsf->setSize(reg2);

            // merge if distance is less than threshold
            if( size1 < minsize || size2 < minsize )
                dsf->join(reg1, reg2);  // merge two regions
        }
    }
}

int SRMSeg :: distance(Vec3b& pix1, Vec3b& pix2)
{
    return MAX3( abs(pix1[0] - pix2[0]),
                 abs(pix1[1] - pix2[1]),
                 abs(pix1[2] - pix2[2]));
}


/// 0 ... numComps-1
void SRMSeg :: getLabels( Mat& labels )
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
void SRMSeg :: getLabelsInt( Mat& labels )
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
void SRMSeg :: drawSegmentBoundaries( Mat& dst, Scalar bcolor )
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
                cv::Mat roi(dst, Rect(x, y, 2, 2));
                roi = bcolor;
                //cvSet2D( dst, y, x+1, bcolor );
                flag = true;
            }

            if(cid != cidy)
            {
                //cvSet2D( dst, y+1, x, bcolor );
                //dst.at<Vec3b>(y+1,x) = bcolor;
                cv::Mat roi(dst, Rect(x, y, 2, 2));
                roi = bcolor;
                flag = true;
            }

            //if(flag)
            //    dst.at<Vec3b>(y,x) = bcolor;
                //cvSet2D( dst, y, x, bcolor );
        }
    }

}
