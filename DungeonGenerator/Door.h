/*
 * Author      : Matthew Johnson
 * Date        : 3/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "MapObject.h"
#include "Inventory.h"

class Door
	: public MapObject
{
public:
	Door( Tile* tile, Mesh* mesh, int keyId=InventoryItem::INVALID_ITEM_ID );
	virtual ~Door();

	void InitPhysics( PhysicsWorld* world );
	void Update( float dt );
	void Use( Actor* user );
	void Lock( int keyId );
	void EvaluateCommands( const std::string& commands );
	
	void SetKeyName( const std::string& name ) { mKeyName = name; }

protected:
	void Open();
	void Close();

	enum
	{
		STATE_CLOSED,
		STATE_OPEN,
		STATE_OPENING,
		STATE_CLOSING
	};
	
	int mDoorState;
	float mDoorOffset;
	float mDoorSpeed;

	int mKeyId;
	std::string mKeyName;
};