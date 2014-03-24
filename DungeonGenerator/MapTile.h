/*
 * Author      : Matthew Johnson
 * Date        : 7/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Object.h"

class Mesh;
struct Tile;

class MapTile
	: public Object
{
public:
	MapTile( Tile* tile );
	virtual ~MapTile();

	void LoadAssets();
	void InitPhysics( PhysicsWorld* world );
	void Draw( Window* window );

protected:
	Mesh* mMesh;
	Tile* mTile;
};