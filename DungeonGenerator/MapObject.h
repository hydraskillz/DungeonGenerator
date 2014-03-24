/*
 * Author      : Matthew Johnson
 * Date        : 8/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Object.h"
#include "Color.h"

class Mesh;
struct Tile;

class MapObject
	: public Object
{
public:
	MapObject( Tile* tile, Mesh* mesh );
	virtual ~MapObject();

	void LoadAssets();
	virtual void InitPhysics( PhysicsWorld* world );
	void Draw( Window* window );

	void SetColor( const Color& color ) { mColor = color; }

protected:
	Mesh* mMesh;
	Tile* mTile;
	Color mColor;
};