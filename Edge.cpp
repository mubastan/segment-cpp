/***************************************************************
 * Name:      Edge.cpp
 * Purpose:   Code for `edge`
 * Author:    Muhammet Bastan (mubastan@gmail.com)
 * Created:   09.07.2010
 * Copyright: Muhammet Bastan (https://sites.google.com/site/mubastan)
 * License:
 **************************************************************/

#include "Edge.h"

/**
 * compare edge weights, needed in STL - sort,
 * edges are sorted according to weights
 */
bool operator<( const edge &a, const edge &b )
{
    return a.w < b.w;
}
