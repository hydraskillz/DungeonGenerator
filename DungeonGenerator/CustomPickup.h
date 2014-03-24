/*
 * Author      : Matthew Johnson
 * Date        : 13/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Pickup.h"

class CustomPickup
	: public Pickup
{
public:
	CustomPickup();
	virtual ~CustomPickup();

	void OnPickup( Actor* actor ) {}
};