/*
 * Author      : Matthew Johnson
 * Date        : 2/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Entity.h"
#include "Inventory.h"
#include "Ammo.h"
#include "glm/glm.hpp"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

//class btCharacterControllerInterface;
//class btBroadphaseInterface;
//class btPairCachingGhostObject;

class PhysicsWorld;

class Actor
	: public Entity
{
public:
	Actor();
	virtual ~Actor();

	// Physics
	void InitPhysics( PhysicsWorld* world );

	inline btCharacterControllerInterface* GetController() const { return mController; }
	inline btPairCachingGhostObject* GetGhostObject() const { return mGhostObject; }

	// World
	glm::vec3 GetPosition() const;
	void Teleport( const glm::vec3& location );
	void SetSpawnLocation( const glm::vec3& location ) { mSpawnLocation = location; }
	void Face( Entity* target );

	Entity* Trace( glm::vec3& hitLoc, glm::vec3& hitNorm, const glm::vec3& end );
	Entity* Trace( glm::vec3& hitLoc, glm::vec3& hitNorm, const glm::vec3& end, const glm::vec3& start );

	// Inventory
	void AddItemToInventory( InventoryItem* item );
	void RemoveItemFromInventory( InventoryItem* item );
	void RemoveItemFromInventory( int type, int id );
	bool HasItem( InventoryItem* item );
	bool HasItem( int type, int id );
	void ClearInventory();

	// Stats
	bool AddLife( int amount, bool overHeal=false );
	bool AddArmor( int amount, bool overHeal=false );
	bool AddAmmo( int amount, int type );
	void Hurt( int amount );
	int GetCurrentLife() const { return mLife; }
	int GetMaxLife() const { return mMaxLife; }
	int GetCurrentArmor() const { return mArmor; }
	int GetMaxArmor() const { return mMaxArmor; }

protected:
	bool AddStat( int amount, int& current, int max, int maxOver, bool over );

	btCharacterControllerInterface* mController;
	btBroadphaseInterface*	mOverlappingPairCache;
	btPairCachingGhostObject* mGhostObject;

	Inventory mInventory;

	int mLife;
	int mMaxLife;
	int mMaxOverHeal;

	int mArmor;
	int mMaxArmor;
	int mMaxOverArmor;

	int mCurrentAmmo[ AMMO_TYPE_COUNT ];
	int mMaxAmmo[ AMMO_TYPE_COUNT ];

	glm::vec3 mSpawnLocation;
};