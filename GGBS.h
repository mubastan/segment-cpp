/***************************************************************
 * Name:      GGBS.h
 * Purpose:   Defines Greedy Graph-Based Segmentation
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   09.07.2010
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#ifndef GGBS_H_INCLUDED
#define GGBS_H_INCLUDED

#include "DisjointSet.h"
#include "Edge.h"

class GGBS
{
    public:
        GGBS( int numNodes, int numEdges, float threshold = 0.5f, int minSize = 5 );
        ~GGBS();

        void allocate( int numNodes, int numEdges );
        void deallocate();
        void setParameters( float threshold, int minsize );

        /// (re)allocates memory if needed and calls reset()
        /// and reset edgeIndex(0)
        /// should be called at the beginning of a new segmentation
        void start(int numNodes, int numEdges);
        /// reset the DSF (segmentation)
        void reset();

        /// add a new edge to the graph
        /// return the current number of edges in the graph, or -1
        /// a and b must be in [0, numNodes-1], or edge is not added & -1 returned
        int addEdge(int a, int b, float weight);

        /// increment amount for edge weight, when joining 2 sets,
        /// used in segmentGraph
        float edgeThresh(int size){ return threshold/size; }
        /// segment the graph into a set of DSFs
        void segmentGraph();
        /// eliminate small regions by merging
        void postProcess();

        /// return class labels for all the nodes (ptr to this->labels)
        /// do not delete the returned pointer!
        int* getLabels();

        /// number of components in the current segmentation
        int getNumComps() const { return ( dsf != NULL ) ? dsf->numSets() : -1; }
        /// size of the set that `id` belongs to
        int getSize(int id) const {return ( dsf != NULL ) ? dsf->setSize(findSet(id)) : -1;}
        /// which set does `id` belong to?
        int findSet(int id) const {return ( dsf != NULL ) ? dsf->find(id) : -1;}


    /// ===== DATA =================================
    public:

        /// number of nodes and edges in the graph to be built
        int numNodes;
        int numEdges;

        /// which edge will be added next
        /// (hence, current number of edges in the graph)
        int edgeIndex;

        /// used in segmentGraph() to decide whether to join two sets
        float threshold;

        /// min segment size (connected component) in the output segmentation
        int minSize;

        /// edge weights, array of size `numEdges`
        edge* edges;

        /// disjoint set forest, total number of elements = `numNodes`
        /// initial number of sets = `numNodes`
        DisjointSet* dsf;

        /// thresholds in segmentGraph, one threshold for each node
        /// array size = `numNodes`
        float* thresholds;

        /// labels of each node, array size = `numNodes`
        /// meaningful after segmentation
        /// range: [0, numNodes]
        int* labels;
};


#endif
