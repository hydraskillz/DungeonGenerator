/*
 * Author      : Matthew Johnson
 * Date        : 5/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Color.h"

#include <vector>

class Window;
class Texture2D;

class InventoryItem
{
public:
	enum ItemType
	{
		ITEM_NONE,
		ITEM_KEY,
	};

	static const int INVALID_ITEM_ID = -1;

	InventoryItem( int type, int id=0 )
		: mType( type )
		, mId( id )
		, mTexture( 0 )
		, mColor( Color::WHITE )
	{}

	bool operator==( const InventoryItem& other ) const
	{
		return mId == other.mId && mType == other.mType;
	}

	void Draw( Window* window, float x, float y );

	int mType;
	int mId;
	Texture2D* mTexture;
	Color mColor;
};

class Inventory
{
public:
	Inventory();
	~Inventory();

	void AddItem( InventoryItem* item );
	void RemoveItem( InventoryItem* item );
	void RemoveItem( int type, int id );
	void RemoveAllItems();
	bool HasItem( InventoryItem* item );
	bool HasItem( int type, int id );

	void Draw( Window* window );

private:
	std::vector< InventoryItem* > mItems;
};