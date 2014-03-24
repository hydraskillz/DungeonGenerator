#include "Object.h"
#include "Util.h"
#include "PhysicsWorld.h"
#include <glm/gtc/matrix_transform.hpp>

//---------------------------------------
Object::Object()
	: mBody( 0 )
{
	mEntityType = ET_WORLD;
	mXForm.setIdentity();
}
//---------------------------------------
Object::~Object()
{
	if ( mBody && mPhysicsWorld )
	{
		mPhysicsWorld->RemoveBody( mBody );
	}
}
//---------------------------------------
void Object::Translate( glm::vec3 tr )
{
	mPosition += tr;
	mXForm.setOrigin( btVector3( mPosition.x, mPosition.y, mPosition.z ) );
	UpdateXForm();
}
//---------------------------------------
void Object::SetLocation( glm::vec3& loc )
{
	mPosition = loc;
	mXForm.setOrigin( btVector3( loc.x, loc.y, loc.z ) );
	UpdateXForm();
}
//---------------------------------------
void Object::SetRotation( glm::vec3& axis, float angle )
{
	mXForm.setRotation( btQuaternion( btVector3( axis.x, axis.y, axis.z ), angle ) );
	UpdateXForm();
}
//---------------------------------------
void Object::UpdateXForm()
{
	if ( mBody ) 
		mBody->setWorldTransform( mXForm );
}
//---------------------------------------
void Object::SetVisible( bool visible )
{
	Entity::SetVisible( visible );

	if ( !mVisible )
	{
		int s = mBody->getActivationState();
		mBody->setActivationState( DISABLE_SIMULATION );
		mBody->setCollisionFlags( mBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
	else
	{
	}
}
//---------------------------------------