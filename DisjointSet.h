#ifndef BIL_DISJOINT_SET_H
#define BIL_DISJOINT_SET_H

// disjoint-set forests using union-by-rank and path compression (sort of).

typedef struct
{
    int rank;
    int p;
    int size;
} uni_elt;

class DisjointSet
{

public:

    DisjointSet( int numElements );
    ~DisjointSet();

    /// reset the DSF, bring it to the initial state
    void reset();

    /// which set does x belong to
    int find( int x );

    /// join sets x, y; do union-by-rank & path compression
    void join( int x, int y );

    /// size of set x
    int setSize( int x ) const { return elts[x].size; }

    /// how many sets are in DSF
    int numSets() const { return count; }

    /// return total number of elements in all sets (total number of nodes)
    int getNumElements() const { return numElements; }


private:

    uni_elt* elts;

    /// number of sets in the DSF
    int count;

    /// total number of elements in all the sets (initial number of sets)
    int numElements;
};


#endif
