#include "Inventory.h"
#include "Util.h"
#include "Window.h"
#include "Texture.h"

//---------------------------------------
void InventoryItem::Draw( Window* window, float x, float y )
{
//	Texture2D::Unbind();
//	window->SetDrawColor( Color( 0.8f, 0.8f, 0.8f, 0.25f ) );
//	window->DrawQuad2D( x - 2.0f, y - 2.0f, 40.0f + 2.0f, 40.0f + 2.0f );

	if ( mTexture )
		mTexture->Bind();
	window->SetDrawColor( mColor );
	window->DrawQuad2D( x, y, 40.0f, 40.0f );
}
//---------------------------------------

//---------------------------------------
Inventory::Inventory()
{}
//---------------------------------------
Inventory::~Inventory()
{
	RemoveAllItems();
}
//---------------------------------------
void Inventory::AddItem( InventoryItem* item )
{
	mItems.push_back( item );
}
//---------------------------------------
void Inventory::RemoveItem( InventoryItem* item )
{
	auto itr = std::find( mItems.begin(), mItems.end(), item );
	if ( itr != mItems.end() )
	{
		SafeDelete( item );
		mItems.erase( itr );
	}
}
//---------------------------------------
void Inventory::RemoveItem( int type, int id )
{
	InventoryItem item( type, id );
	for ( unsigned i = 0; i < mItems.size(); ++i )
	{
		if ( *mItems[i] == item )
		{
			RemoveItem( mItems[i] );
			break;
		}
	}
}
//---------------------------------------
void Inventory::RemoveAllItems()
{
	for ( auto itr = mItems.begin(); itr != mItems.end(); ++itr )
		delete *itr;
	mItems.clear();
}
//---------------------------------------
bool Inventory::HasItem( InventoryItem* item )
{
	auto itr = std::find( mItems.begin(), mItems.end(), item );
	return itr != mItems.end();
}
//---------------------------------------
bool Inventory::HasItem( int type, int id )
{
	InventoryItem item( type, id );
	for ( unsigned i = 0; i < mItems.size(); ++i )
	{
		if ( *mItems[i] == item )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
void Inventory::Draw( Window* window )
{
	float x = window->GetWidth() - 45.0f;
	float y = 35.0f;
	window->SetOrtho();
	for ( auto itr = mItems.begin(); itr != mItems.end(); ++itr )
	{
		(*itr)->Draw( window, x, y );
		y += 42.0f;
	}
	window->SetPerspective();
}
//---------------------------------------