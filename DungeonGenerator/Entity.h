/*
 * Author      : Matthew Johnson
 * Date        : 2/Feb/2014
 * Description :
 *   Base for all things in the world.
 */
 
#pragma once

#include "XmlReader.h"
#include "EventListener.h"

#include <vector>
#include <glm/glm.hpp>

class Window;
class Texture2D;
class PhysicsWorld;
class Actor;
class EntityFactory;

class Entity
	: public EventListener
{
public:
	// Used by physics to signal static geometry was hit
	static Entity WORLD;

	// Factory functions
	template< typename EntityT >
	static void LoadTemplatesFromXml( const XmlReader::XmlReaderIterator& xmlItr )
	{
		EntityT::GetFactory()->LoadTemplatesFromXml( xmlItr );
	}

	template< typename EntityT >
	static EntityT* CreateEntity( const std::string& tempalteName )
	{
		return EntityT::GetFactory()->CreateEntity( tempalteName );
	}

	// Type this entity is
	enum EntityType
	{
		ET_NONE,		// 
		ET_WORLD,		// World geometry
		ET_PICKUP,		// Pickup
		ET_ACTOR,		// Actor/Player/Enemy
		ET_WEAPON,		// Weapon
		ET_COUNT,		//
	};

	enum RenderGroup
	{
		RG_SCENE,
		RG_FOREGROUND,
	};
	
	Entity();
	virtual ~Entity();

	// Entities are initialed by the game in this order:
	//  LoadAssets()
	//  Initialize()
	//  InitPhysics()

	virtual void Initialize() {}
	virtual void InitPhysics( PhysicsWorld* world ) { mPhysicsWorld = world; }
	virtual void LoadAssets();
	virtual void Update( float dt );
	virtual void Draw( Window* window );
	void SetParent( Entity* parent ) { mParent = parent; }
	// Attachments will share lifespan with this entity
	void AddAttachment( Entity* attachment ) { mAttachments.push_back( attachment ); }
	// Player pressed action while aiming at this entity
	virtual void Use( Actor* user ) { FireSignal( SIGNAL_ON_USE ); }
	// Signal the entity should be removed from the scene next frame
	virtual bool WantsDeletion() { return mDestroyed; }
	// Call when you want this entity removed from the scene
	// All attachments will be removed as well
	void Destroy();
	virtual void OnDestroy() {}
	virtual void Kill() { mAlive = false; FireSignal( SIGNAL_ON_DEATH ); }
	inline bool IsAlive() const { return mAlive; }
	virtual void SetVisible( bool visible ) { mVisible = visible; }
	// Called when two entities collide
	// Return false to have other->OnCollision( this ) be called
	virtual bool OnCollision( Entity* other ) { return false; }
	short GetType() const { return mEntityType; }
	short GetRenderGroup() const { return mRenderGroup; }
	virtual glm::vec3 GetPosition() const { return glm::vec3( 0 ); }
	virtual void TakeDamage( int damage, Entity* instigator ) {}
	const std::string& GetName() const { return mName; }
	virtual void EvaluateCommands( const std::string& commands ) {};

	bool operator< ( const Entity& other ) const
	{
		return mEntityType < other.mEntityType;
	}

private:
	static EntityFactory* GetFactoty();


protected:
	std::string mName;
	PhysicsWorld* mPhysicsWorld;
	Entity* mParent;
	std::vector< Entity* > mAttachments;
	short mEntityType;
	short mRenderGroup;
	bool mAlive;
	bool mVisible;
private:
	bool mDestroyed;
};