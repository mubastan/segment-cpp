
#include "DisjointSet.h"

/**
 * constructor: create a disjoint set forest, numElements sets,
 * each set has size 1, rank 0, parent itself initially
 */
DisjointSet :: DisjointSet( int numElements )
{

    elts = new uni_elt[ numElements ];
    this -> count = numElements;

    // initial number of elements, stored for resetting
    this -> numElements = numElements;

    for (int i = 0; i < numElements; i++)
    {
        elts[i].rank = 0;
        elts[i].size = 1;
        elts[i].p = i;
    }
}

/**
 * destructor, releases memory
 */
DisjointSet :: ~DisjointSet()
{
    if( elts)
        delete [] elts;
    elts = 0;
}

/**
 * find which set `x` belongs to (i.e., the root)
 */
int DisjointSet :: find( int x )
{
    int y = x;
    while ( y != elts[y].p )
        y = elts[y].p;

    elts[x].p = y;

    return y;
}

/**
 * join/unite 2 sets x, y (union by rank)
 */
void DisjointSet :: join( int x, int y ) {

    // added, 05.07.2007
    if( x == y )
      return;

    if ( elts[x].rank > elts[y].rank )
    {
        elts[y].p = x;
        elts[x].size += elts[y].size;
    }
    else
    {
        elts[x].p = y;
        elts[y].size += elts[x].size;

        if ( elts[x].rank == elts[y].rank )
            elts[y].rank++;
    }

    // decrement number of sets, since we joined 2 of them
    count --;
}

/**
 * reset the DSF, bring it to the initial state
 */
void DisjointSet :: reset()
{

  this -> count = this->numElements;

    for (int i = 0; i < this->numElements; i++)
    {
        elts[i].rank = 0;
        elts[i].size = 1;
        elts[i].p = i;
    }

}
