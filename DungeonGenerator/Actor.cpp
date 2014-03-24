#include "Actor.h"

#include "Util.h"
#include "PhysicsWorld.h"

//---------------------------------------
Actor::Actor()
	: mController( 0 )
	, mGhostObject( 0 )
	, mLife( 75 )
	, mMaxLife( 100 )
	, mMaxOverHeal( 200 )
	, mArmor( 0 )
	, mMaxArmor( 100 )
	, mMaxOverArmor( 200 )
{
	mEntityType = ET_ACTOR;

	mMaxAmmo[ AMMO_BULLET ] = 500;
	mCurrentAmmo[ AMMO_BULLET ] = 150;
}
//---------------------------------------
Actor::~Actor()
{
	if ( mPhysicsWorld )
	{
		mPhysicsWorld->RemoveActor( this );
	}
	SafeDelete( mController );
	SafeDelete( mGhostObject );
}
//---------------------------------------
void Actor::InitPhysics( PhysicsWorld* world )
{
	Entity::InitPhysics( world );

	btTransform startTransform;
	startTransform.setIdentity();

	mGhostObject = new btPairCachingGhostObject();
	mGhostObject->setWorldTransform(startTransform);
	mGhostObject->setUserPointer( this );
	
	btScalar characterHeight=0.175f;
	btScalar characterWidth =0.25f;
	btConvexShape* capsule = new btCapsuleShape(characterWidth,characterHeight);
	mGhostObject->setCollisionShape( capsule );
	mGhostObject->setCollisionFlags( btCollisionObject::CF_CHARACTER_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

	btScalar stepHeight = btScalar(0.35);
	mController = new btKinematicCharacterController (mGhostObject,capsule,stepHeight);

	world->AddActor( this );

	Teleport( mSpawnLocation );
}
//---------------------------------------
glm::vec3 Actor::GetPosition() const
{
	btVector3 pos = mGhostObject->getWorldTransform().getOrigin();
	return glm::vec3( pos.x(), pos.y(), pos.z() );
}
//---------------------------------------
void Actor::Teleport( const glm::vec3& location )
{
	mController->warp( btVector3( location.x, location.y, location.z ) );
}
//---------------------------------------
void Actor::Face( Entity* target )
{
	glm::vec3 originPos = GetPosition();
	glm::vec3 targetPos = target->GetPosition();
	glm::vec3 directionToTarget = targetPos - originPos;
	directionToTarget = glm::normalize( directionToTarget );
	glm::vec3 axis( 1, 0, 0 );
	float angle =std::atan2( directionToTarget.x, directionToTarget.z ) - std::atan2( axis.x, axis.z );
	btQuaternion rot( btVector3( 0, 1, 0 ), angle );
	mGhostObject->getWorldTransform().setRotation( rot );
}
//---------------------------------------
Entity* Actor::Trace( glm::vec3& hitLoc, glm::vec3& hitNorm, const glm::vec3& end )
{
	return Trace( hitLoc, hitNorm, end, GetPosition() );
}
//---------------------------------------
Entity* Actor::Trace( glm::vec3& hitLoc, glm::vec3& hitNorm, const glm::vec3& end, const glm::vec3& start )
{
	return mPhysicsWorld->RaycastClosestNotMe( 
		end,
		start,
		mGhostObject,
		hitNorm,
		hitLoc );
}
//---------------------------------------
void Actor::AddItemToInventory( InventoryItem* item )
{
	mInventory.AddItem( item );
}
//---------------------------------------
void Actor::RemoveItemFromInventory( InventoryItem* item )
{
	mInventory.RemoveItem( item );
}
//---------------------------------------
void Actor::RemoveItemFromInventory( int type, int id )
{
	mInventory.RemoveItem( type, id );
}
//---------------------------------------
bool Actor::HasItem( InventoryItem* item )
{
	return mInventory.HasItem( item );
}
//---------------------------------------
bool Actor::HasItem( int type, int id )
{
	return mInventory.HasItem( type, id );
}
//---------------------------------------
void Actor::ClearInventory()
{
	mInventory.RemoveAllItems();
}
//---------------------------------------
bool Actor::AddLife( int amount, bool overHeal )
{
	return AddStat( amount, mLife, mMaxLife, mMaxOverHeal, overHeal );
}
//---------------------------------------
bool Actor::AddArmor( int amount, bool overHeal )
{
	return AddStat( amount, mArmor, mMaxArmor, mMaxOverArmor, overHeal );
}
//---------------------------------------
bool Actor::AddAmmo( int amount, int type )
{
	return AddStat( amount, mCurrentAmmo[ type ], mMaxAmmo[ type ], 0, false );
}
//---------------------------------------
bool Actor::AddStat( int amount, int& current, int max, int maxOver, bool over )
{
	bool healed = false;

	if ( amount == 0 )
		return false;

	// Already overhealed
	if ( current >= max )
	{
		// Can only gain more life if healing is overHeal
		if ( over )
		{
			current += amount;
			healed = true;
			// Clamp to MaxOverHeal
			if ( current > maxOver )
				current = maxOver;
		}
	}
	// < maxLife
	else
	{
		current += amount;
		healed = true;

		// This healing can overheal
		if ( over )
		{
			if ( current > maxOver )
				current = maxOver;
		}
		// Clamp to maxLife
		else
		{
			if ( current > max )
				current = max;
		}
	}

	return healed;
}
//---------------------------------------
void Actor::Hurt( int amount )
{
	mLife -= amount;
	if ( mLife < 0 )
		mLife = 0;
	//printf( "Life = %d\n" , mLife );
	if ( mLife == 0 && IsAlive() )
	{
		Kill();

		if ( mPhysicsWorld )
		{
			mPhysicsWorld->RemoveActor( this );
		}
	}
}
//---------------------------------------