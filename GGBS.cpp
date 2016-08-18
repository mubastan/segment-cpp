/***************************************************************
 * Name:      GGBS.cpp
 * Purpose:   Code for Greedy Graph-Based Segmentation
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   09.07.2010
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "GGBS.h"

GGBS::GGBS( int numNodes, int numEdges, float threshold, int minsize )
{
    this->minSize = 1;
    this->threshold = 0.50f;

    edgeIndex = 0;

    setParameters( threshold, minsize );

    // set pointer to NULL
    this->edges = NULL;
    this->dsf = NULL;
    this->thresholds = NULL;
    this->labels = NULL;

    allocate( numNodes, numEdges );
}

GGBS::~GGBS()
{
    deallocate();
}

void GGBS :: allocate( int numNodes, int numEdges )
{
	if( numNodes < 1 || numEdges < 1 )
        throw "GGBS :: allocate: numNodes and numEdges must be > 0!";

	this->numNodes = numNodes;
	this->numEdges = numEdges;

	// will keep the graph edges
    this -> edges = new edge[ numEdges ];

    // DSF, numVertices
    this -> dsf = new DisjointSet( numNodes );

    // thresholds array, one threshold for each node
    this -> thresholds = new float[ numNodes ];

    // class labels for the nodes
    this->labels = new int[ numNodes ];

    if( !this -> edges || !this -> dsf || !this -> thresholds || !this->labels)
        throw "GGBS :: allocate: Memory allocation failed!";

}

void GGBS :: deallocate()
{
	if( edges )
        delete[] edges;
    edges = NULL;

    if( dsf )
        delete dsf;
    dsf = NULL;

    if( thresholds )
        delete[] thresholds;
    thresholds = NULL;

    if( labels )
        delete [] labels;
    labels = NULL;
}

void GGBS :: setParameters( float threshold, int minsize )
{
    if(minsize > 0)
        this->minSize = minsize;

	if (threshold > 0)
        this->threshold = threshold;
}

void GGBS :: reset()
{
    if(dsf)
    {
        dsf->reset();
    }
    else
        throw "GGBS :: reset: no DSF to reset!";
}

void GGBS :: start(int numNodes, int numEdges)
{
    if( numNodes != this->numNodes ||  numEdges != this->numEdges)
    {
        this->deallocate();
		this->allocate( numNodes,  numEdges);
    }

    reset();
    this->edgeIndex = 0;
}

/**
 * add a new edge to the graph
 * a and b must be in [0, numNodes-1], or edge is not added & -1 returned
 */
int GGBS :: addEdge(int a, int b, float weight)
{
    if( a >= this->numNodes || b >= this->numNodes || a < 0 ||  b < 0  )
        return -1;

    this->edges[this->edgeIndex].a = a;
    this->edges[this->edgeIndex].b = b;
    this->edges[this->edgeIndex].w = weight;

    this->edgeIndex++;

    return this->edgeIndex;
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
void  GGBS :: segmentGraph()
{
    // number of edges currently available in the graph
    int numEdges = this->edgeIndex;

    // sort edges by weight
    std::sort(edges, edges + numEdges );

    // initialize thresholds for each node
    int i;
    for (i = 0; i < numNodes; i++)
	    thresholds[i] = this->threshold;    // eguiv. to: threshold/1


    // for each edge, in non-decreasing weight order...
    edge* pedge = this->edges;
    for (i = 0; i < numEdges; i++, pedge++)
    {
	    //pedge = &edges[i];

		// components conected by this edge
		int a = dsf -> find( pedge->a );
		int b = dsf -> find( pedge->b );
		if ( a != b )
		{
		    if ( ( pedge->w <= thresholds[a] ) && ( pedge->w <= thresholds[b] ) )
		    {
			    dsf->join(a, b);
				a = dsf->find(a);
				thresholds[a] = pedge->w + edgeThresh( dsf->setSize(a) );
		    }
	    }
    }
}

void GGBS :: postProcess()
{
    int i, a, b;
    // post process small components
    edge* pedge = this->edges;
    for ( i = 0; i < this->edgeIndex; i++, pedge++ )
    {
        a = dsf->find( pedge->a );
        b = dsf->find( pedge->b );
        if ( (a != b) && ( ( dsf->setSize(a) <= minSize ) || ( dsf->setSize(b) <= minSize )))
            dsf->join(a, b);
    }
}

/**
 * return class labels for all the nodes
 * labels are in the range [0, numNodes]
 */
int* GGBS :: getLabels()
{
    if(!this->dsf)
        return 0;

    if(!this->labels)
        this->labels = new int[numNodes];

    int* ltemp = this->labels;
    for( int i = 0; i < numNodes; i++, ltemp++ )
    {
        *ltemp = dsf->find(i);
    }

    return this->labels;
}
