#include "Camera.h"
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

//---------------------------------------
Camera::Camera()
	: horizontalAngle( 3.14f )
	, verticalAngle( 0.0f )
	, FoV( 45.0f )
	, aspectRatio( 4.0f / 3.0f )
	, currentSpeed( 0 )
	, baseSpeed( 6.0f )
	, sprintMultiplier( 1.625f )
	, mouseSpeed( 0.005f )
	, map( 0 )
	, ghost( false )
{}
//---------------------------------------
Camera::~Camera()
{}
//---------------------------------------
void Camera::ComputeFromInput( float dt, bool left, bool right, bool forward, bool backward, bool up, bool down, bool sprint, int mx, int my )
{
	SDL_WarpMouse( width / 2, height / 2 );

	// Compute new orientation
	horizontalAngle += mouseSpeed * float( width / 2 - mx );
	verticalAngle   += mouseSpeed * float( height / 2 - my );

	// Clamp to [-90,90]
	if ( verticalAngle > 1.57f )
		verticalAngle = 1.57f;
	if ( verticalAngle < -1.57f )
		verticalAngle = -1.57f;

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	// Right vector
	glm::vec3 vright = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
		);

	// Up vector
	glm::vec3 vup = glm::cross( vright, direction );

	float speed = baseSpeed;
	if ( sprint )
		speed *= sprintMultiplier;
	currentSpeed = speed;

	if ( ghost )
	{
		// Move forward
		if ( forward ){
			position += direction * dt * speed;
		}
		// Move backward
		if ( backward ){
			position -= direction * dt * speed;
		}
		// Strafe right
		if ( right ){
			position += vright * dt * speed;
		}
		// Strafe left
		if ( left ){
			position -= vright * dt * speed;
		}

		// Up
		if ( up )
			position.y += dt * speed;
		// Down
		if ( down )
			position.y -= dt * speed;
	}

	// Projection matrix
	projectionMatrix = glm::perspective(FoV, aspectRatio, 0.1f, 100.0f);
	// Camera matrix
	viewMatrix       = glm::lookAt(
		position,           // Camera is here
		position+direction, // and looks here : at the same position, plus "direction"
		vup                  // Head is up (set to 0,-1,0 to look upside-down)
		);
}
//---------------------------------------
void Camera::LookAt( const glm::vec3& pos )
{
	viewMatrix       = glm::lookAt(
		position,
		pos,
		glm::vec3( 0, 1, 0 )
	);
}
//---------------------------------------