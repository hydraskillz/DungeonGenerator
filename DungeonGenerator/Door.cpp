#include "Door.h"
#include "Util.h"
#include "Window.h"
#include "Texture.h"
#include "PhysicsWorld.h"
#include "Actor.h"
#include "Mesh.h"
#include "GameLog.h"
#include "Logger.h"
#include "Game.h"

#include <glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"

//---------------------------------------
Door::Door( Tile* tile, Mesh* mesh, int keyId )
	: MapObject( tile, mesh )
	, mDoorState( STATE_CLOSED )
	, mDoorOffset( 0 )
	, mDoorSpeed( 2.0f )
	, mKeyId( keyId )
{}
//---------------------------------------
Door::~Door()
{}
//---------------------------------------
void Door::InitPhysics( PhysicsWorld* world )
{
	MapObject::InitPhysics( world );

	mBody->setUserPointer( this );
}
//---------------------------------------
void Door::Update( float dt )
{
	if ( mDoorState == STATE_OPENING )
	{
		float delta = mDoorSpeed * dt;
		mDoorOffset += delta;
		if ( mDoorOffset >= 1.0f )
		{
			mDoorOffset = 1.0f;
			mDoorState = STATE_OPEN;
		}
		Translate( glm::vec3( 0, delta, 0 ) );
		
	}
	else if ( mDoorState == STATE_CLOSING )
	{
		float delta = mDoorSpeed * dt;
		mDoorOffset -= delta;
		if ( mDoorOffset <= 0.0f )
		{
			mDoorOffset = 0.0f;
			mDoorState = STATE_OPEN;
		}
		Translate( glm::vec3( 0, -delta, 0 ) );
	}
}
//---------------------------------------
void Door::Use( Actor* user )
{
	DebugPrintf( "Used Door!\n" );

	if ( mKeyId != InventoryItem::INVALID_ITEM_ID )
	{
		if ( user->HasItem( InventoryItem::ITEM_KEY, mKeyId ) )
		{
			GameLog::Instance.PostMessageFmt( "Used %s\n", mKeyName.c_str() );
			// Can't safely take their key away without doing a ref count on all doors that use the key (is usually 1, but may be more in some patterns)
			//user->RemoveItemFromInventory( InventoryItem::ITEM_KEY, mKeyId );
			mTile->mLocked = false;
			Open();
		}
		else
		{
			if ( !mKeyName.empty() )
			{
				GameLog::Instance.PostMessageFmt( "Requires %s\n", mKeyName.c_str() );
			}
			else
			{
				GameLog::Instance.PostMessageFmt( "The door is sealed shut.\n" );
			}
		}
	}
	else
	{
		Open();
	}
}
//---------------------------------------
void Door::Lock( int keyId )
{
	mKeyId = keyId;
}
//---------------------------------------
void Door::Open()
{
	if ( mDoorState == STATE_CLOSED || mDoorState == STATE_CLOSING )
	{
		mDoorState = STATE_OPENING;
		// This is so the minimap can update line of sight past the door tile
		mTile->mType = Tile::Tile_FLOOR;
	}
}
//---------------------------------------
void Door::Close()
{
	if ( mDoorState == STATE_OPEN || mDoorState == STATE_OPENING )
	{
		mDoorState = STATE_CLOSING;
		// This is so the minimap can update line of sight past the door tile
		mTile->mType = Tile::Tile_DOOR;
	}
}
//---------------------------------------
void Door::EvaluateCommands( const std::string& commands )
{
	if ( commands == "open" )
		Open();
	else if ( commands == "close" )
		Close();
}
//---------------------------------------