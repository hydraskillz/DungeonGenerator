#pragma once

#include <stdlib.h>
#include <iostream>
#include <algorithm>

#include "WeightedRandom.h"

#include "glm/glm.hpp"
#include "LinearMath/btVector3.h"

#define SafeDelete( ptr ) if ( ptr ) { delete ptr; ptr = 0; }

inline glm::vec3 btVect3ToglmVec3( const btVector3& v )
{
	return glm::vec3( v.x(), v.y(), v.z() );
}

inline int GetManhattanDistance( int sx, int sy, int ex, int ey)
{
	int dx = ex - sx;
	int dy = ey - sy;
	return std::abs( dx ) + std::abs( dy );
}