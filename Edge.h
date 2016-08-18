/***************************************************************
 * Name:      Edge.h
 * Purpose:   Defines and edge in a graph
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   09.07.2010
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#ifndef EDGE_H_INCLUDED
#define EDGE_H_INCLUDED

/// an edge in a graph
typedef struct
{
    float w;
    int a;
    int b;
} edge;

//std::ostream& operator<<( std::ostream& ostr, const edge& e );
bool operator<( const edge &a, const edge &b );

#endif
