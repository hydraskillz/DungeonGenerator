/*
 * Author      : Matthew Johnson
 * Date        : 4/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Object.h"

class Decal
	: public Object
{
public:
	Decal( float life=10.0f );
	virtual ~Decal();

	void LoadAssets();
	void Update( float dt );
	void Draw( Window* window );

protected:
	Texture2D* mDecalTexture;
	float mLife;
};