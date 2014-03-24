#include "Entity.h"
#include "EntityFactory.h"

class EntityTemplate
	: public EntityTemplateBase
{};

class EntityFactory
	: public EntityFactoryBase< Entity, EntityTemplate >
{
public:
	void InternalCreateEntity( Entity& entity, const EntityTemplate& entityTmpl )
	{
	}
};

Entity Entity::WORLD;

//---------------------------------------
EntityFactory* GetFactoty()
{
	static EntityFactory factory;
	return &factory;
}
//---------------------------------------


//---------------------------------------
Entity::Entity()
	: mPhysicsWorld( 0 )
	, mParent( 0 )
	, mAlive( true )
	, mVisible( true )
	, mDestroyed( false )
	, mEntityType( ET_NONE )
	, mRenderGroup( RG_SCENE )
{}
//---------------------------------------
Entity::~Entity()
{}
//---------------------------------------
void Entity::LoadAssets()
{
}
//---------------------------------------
void Entity::Update( float dt )
{
}
//---------------------------------------
void Entity::Draw( Window* window )
{
}
//---------------------------------------
void Entity::Destroy()
{
	 OnDestroy();
	 mDestroyed = true; 

	 for ( auto itr = mAttachments.begin(); itr != mAttachments.end(); ++itr )
	 {
		 (*itr)->Destroy();
	 }
}
//---------------------------------------