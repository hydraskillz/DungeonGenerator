/*
 * Author      : Matthew Johnson
 * Date        : 4/Feb/2014
 * Description :
 *   An Entity with a position in the world and a Physics body.
 */
 
#pragma once

#include "Entity.h"
#include <glm/glm.hpp>
#include "LinearMath/btTransform.h"

class btRigidBody;

class Object
	: public Entity
{
public:
	Object();
	virtual ~Object();

	void SetTransform( const glm::mat4& tm ) { mTransform = tm; }
	const glm::mat4& GetTransform() const { return mTransform; }

	void Translate( glm::vec3 tr );
	void SetLocation( glm::vec3& loc );
	void SetRotation( glm::vec3& axis, float angle );

	glm::vec3 GetPosition() const { return mPosition; }

	void UpdateXForm();
	void SetVisible( bool visible );

protected:
	glm::mat4 mTransform;
	glm::vec3 mPosition;

	btTransform mXForm;
	btRigidBody* mBody;
};