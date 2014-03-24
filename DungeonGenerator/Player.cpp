#include "Player.h"
#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "PhysicsWorld.h"
#include "Util.h"
#include "Pickup.h"
#include "Weapon.h"

//---------------------------------------
Player::Player()
	: mForward( 0 )
	, mBackward( 0 )
	, mRotation( 0 )
	, mStrafeLeft( 0 )
	, mStrafeRight( 0 )
	, mActiveWeaponIndex( 0 )
{}
//---------------------------------------
Player::~Player()
{}
//---------------------------------------
void Player::Rotate( float radians )
{
	mRotation -= radians;
}
//---------------------------------------
void Player::Update( float dt )
{
	if ( mCamera->ghost )
	{
		mController->setWalkDirection( btVector3(0,0,0) );
		return;
	}

	//rotate view
	mGhostObject->getWorldTransform().setBasis(
		btMatrix3x3(btQuaternion(btVector3(0,1,0),mRotation)));
	
	///set walkDirection for our character
	btTransform xform;
	xform = mGhostObject->getWorldTransform ();

	btVector3 forwardDir = xform.getBasis()[2];
	//	printf("forwardDir=%f,%f,%f\n",forwardDir[0],forwardDir[1],forwardDir[2]);
	btVector3 upDir = xform.getBasis()[1];
	btVector3 strafeDir = xform.getBasis()[0];
	forwardDir.normalize ();
	upDir.normalize ();
	strafeDir.normalize ();

	btVector3 walkDirection = btVector3(0.0, 0.0, 0.0);
	btScalar walkVelocity = mCamera->currentSpeed; // 4 km/h -> 1.1 m/s
	btScalar walkSpeed = walkVelocity * dt;

	
	/*if (gLeft)
	{
		btMatrix3x3 orn = mGhostObject->getWorldTransform().getBasis();
		orn *= btMatrix3x3(btQuaternion(btVector3(0,1,0),0.01));
		mGhostObject->getWorldTransform ().setBasis(orn);
	}

	if (gRight)
	{
		btMatrix3x3 orn = mGhostObject->getWorldTransform().getBasis();
		orn *= btMatrix3x3(btQuaternion(btVector3(0,1,0),-0.01));
		mGhostObject->getWorldTransform ().setBasis(orn);
	}*/

	if (mForward)
		walkDirection -= forwardDir;

	if (mBackward)
		walkDirection += forwardDir;

	if (mStrafeLeft)
		walkDirection -= strafeDir;

	if (mStrafeRight)
		walkDirection += strafeDir;


	mController->setWalkDirection(walkDirection*walkSpeed);
	
	mCamera->position = GetPosition();
	mCamera->position.y += 0.15f;
}
//---------------------------------------
void Player::Draw( Window* window )
{
	mInventory.Draw( window );
	/*btVector3 v = mGhostObject->getWorldTransform().getOrigin();
	window->DrawCube( glm::vec3( v.x(), v.z(), v.y() ) );*/
}
//---------------------------------------
bool Player::IsMoving() const
{
	return mForward + mBackward + mStrafeLeft + mStrafeRight != 0;
}
//---------------------------------------
bool Player::GiveWeapon( Weapon* weapon )
{
	std::pair< std::set< std::string >::iterator, bool > ret = mWeaponsByName.insert( weapon->GetName() );
	if ( ret.second )
	{
		Game::Get()->AddEntity( weapon );
		weapon->SetOwner( this );
		mWeapons.push_back( weapon );
		mActiveWeaponIndex = mWeapons.size() - 1;
		EquipWeapon();
	}
	else
	{
		delete weapon;
	}

	return ret.second;
}
//---------------------------------------
void Player::NextWeapon()
{
	if ( mWeapons.size() > 0 )
	{
		mActiveWeaponIndex = ( mActiveWeaponIndex + 1 ) % mWeapons.size();
		EquipWeapon();
	}
}
//---------------------------------------
void Player::PrevWeapon()
{
	if ( mWeapons.size() > 0 )
	{
		mActiveWeaponIndex = ( mActiveWeaponIndex - 1 ) % mWeapons.size();
		EquipWeapon();
	}
}
//---------------------------------------
void Player::EquipWeapon()
{
	for ( unsigned i = 0; i < mWeapons.size(); ++i )
	{
		if ( i == mActiveWeaponIndex )
		{
			mWeapons[ i ]->SetVisible( true );
		}
		else
		{
			mWeapons[ i ]->SetVisible( false );
		}
		mWeapons[ i ]->StopFire();
	}
}
//---------------------------------------
void Player::InitializeWeapons()
{
	mWeapons.clear();
//	mActiveWeaponIndex = 0;
	for ( auto itr = mWeaponsByName.begin(); itr != mWeaponsByName.end(); ++itr )
	{
		Weapon* wep = Weapon::CreateWeapon( *itr );
		Game::Get()->AddEntity( wep );
		wep->SetOwner( this );
		mWeapons.push_back( wep );
		EquipWeapon();
	}
}
//---------------------------------------
void Player::StartFire()
{
	if ( mActiveWeaponIndex < mWeapons.size() )
		mWeapons[ mActiveWeaponIndex ]->StartFire();
}
//---------------------------------------
void Player::StopFire()
{
	if ( mActiveWeaponIndex < mWeapons.size() )
		mWeapons[ mActiveWeaponIndex ]->StopFire();
}
//---------------------------------------
void Player::HideHUD( bool hide )
{
	for ( unsigned i = 0; i < mWeapons.size(); ++i )
	{
		if ( hide )
			mWeapons[ i ]->SetVisible( false );
		else
		{
			if ( i == mActiveWeaponIndex )
			{
				mWeapons[ i ]->SetVisible( true );
			}
			else
			{
				mWeapons[ i ]->SetVisible( false );
			}
		}
	}
}
//---------------------------------------
void Player::UseAction()
{
	glm::vec3 hitNormal, hitLocation;
	glm::vec3 forward = mCamera->direction;
	forward *= 0.75f;
	Entity* e = mPhysicsWorld->RaycastClosestNotMe( mCamera->position + forward, mCamera->position, GetGhostObject(), hitNormal, hitLocation );
	if ( e )
		e->Use( this );
}
//---------------------------------------
Entity* Player::Raycast( glm::vec3& hitNormal, glm::vec3& hitLocation )
{
	glm::vec3 forward = mCamera->direction;
	forward *= 10;
	return mPhysicsWorld->RaycastClosestNotMe( mCamera->position + forward, mCamera->position, GetGhostObject(), hitNormal, hitLocation );

/*	///set walkDirection for our character
	btTransform xform;
	xform = mGhostObject->getWorldTransform ();

	btVector3 forwardDir = xform.getBasis()[2];
	forwardDir.normalize ();

	glm::vec3 end = btVect3ToglmVec3( xform.getOrigin() ) - btVect3ToglmVec3( forwardDir ) * 10.0f;
	return Trace( hitLocation, hitNormal, end );*/
}
//---------------------------------------
bool Player::OnCollision( Entity* other )
{
	if ( !other->IsAlive() ) return false;
	if ( other->GetType() == ET_PICKUP )
	{
		((Pickup*) other)->OnPickup( this );
	}
	return true;
}
//---------------------------------------