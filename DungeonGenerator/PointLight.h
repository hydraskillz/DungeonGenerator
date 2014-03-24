/*
 * Author      : Matthew Johnson
 * Date        : 15/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Entity.h"
#include "Color.h"

class PointLight
	: public Entity
{
public:
	PointLight();
	virtual ~PointLight();

	glm::vec3 GetPosition() const { return mPosition; }

	Color mColor;
	float mIntensity;
	float mRadius;
	float mFalloff;
	glm::vec3 mPosition;
};