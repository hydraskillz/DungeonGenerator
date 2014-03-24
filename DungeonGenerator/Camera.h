/*
 * Author      : Matthew Johnson
 * Date        : 23/Jan/2014
 * Description :
 *   
 */
 
#pragma once

#include <glm/glm.hpp>
#include "DungeonGenerator.h"

class Camera
{
public:
	Camera();
	~Camera();

	void ComputeFromInput( float dt, bool left, bool right, bool forward, bool backward, bool up, bool down, bool sprint, int mx, int my );
	void LookAt( const glm::vec3& pos );

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 direction;
	float horizontalAngle;
	float verticalAngle;
	float FoV;
	float aspectRatio;
	float currentSpeed;
	float baseSpeed;
	float sprintMultiplier;
	float mouseSpeed;
	int width, height;
	bool ghost;
	DungeonGenerator* map;
};