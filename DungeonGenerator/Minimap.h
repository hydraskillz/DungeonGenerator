/*
 * Author      : Matthew Johnson
 * Date        : 1/Feb/2014
 * Description :
 *   
 */
 
#pragma once

class DungeonGenerator;
class Window;

class Minimap
{
public:
	Minimap();
	~Minimap();

	void SetMap( DungeonGenerator* map ) { mMap = map; }
	void SetSize( float w, float h ) { mWidth = w, mHeight = h; }
	void SetCenter( float x, float y, float a ) { mCenterX = x; mCenterY = y; mRotation = a; }
	void SetCellSize( float s ) { mCellSize = s; }
	void Draw( Window* window );
	void UpdateVisibility( float range );
	void RevealAll();

private:
	float mWidth, mHeight;
	float mCenterX, mCenterY;
	float mRotation;
	float mCellSize;
	DungeonGenerator* mMap;
};