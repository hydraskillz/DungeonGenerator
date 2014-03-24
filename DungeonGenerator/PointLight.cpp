#include "PointLight.h"
#include "Game.h"

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
	// Remove the light from the light list
	Game::Get()->DestroyLight( this );
}