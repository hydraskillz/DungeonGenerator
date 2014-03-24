#include "Minimap.h"
#include "Window.h"
#include "DungeonGenerator.h"
#include "Texture.h"
#include "Plotter.h"

//---------------------------------------
Minimap::Minimap()
	: mMap( 0 )
	, mWidth( 200.0f )
	, mHeight( 200.0f )
	, mCenterX( 0 )
	, mCenterY( 0 )
	, mCellSize( 10 )
{}
//---------------------------------------
Minimap::~Minimap()
{}
//---------------------------------------
void Minimap::Draw( Window* window )
{
	Texture2D::Unbind();
	window->SetOrtho();
	window->SetDrawColor( Color( 0, 0, 0, 0.33f ) );
	window->DrawQuad2D( 0, 0, mWidth, mHeight );
	window->SetScissorRegion( 0, 0, (int) mWidth, (int) mHeight );
	window->EnableScissor();
	int cx = (int) ( mWidth / ( 2 * mCellSize ) );
	int cy = (int) ( mHeight / ( 2 * mCellSize ) );
	int sx = (int) ( mCenterX - cx );
	int sy = (int) ( mCenterY - cy );
	int ex = (int) ( sx + mWidth / mCellSize );
	int ey = (int) ( sy + mHeight / mCellSize );

	if ( sx < 0 ) sx = 0;
	if ( sy < 0 ) sy = 0;
	if ( sx > mMap->GetWidth() ) sx = mMap->GetWidth();
	if ( sy > mMap->GetHeight() ) sy = mMap->GetHeight();

	if ( ex < 0 ) ex = 0;
	if ( ey < 0 ) ey = 0;
	if ( ex > mMap->GetWidth() ) ex = mMap->GetWidth();
	if ( ey > mMap->GetHeight() ) ey = mMap->GetHeight();

	for ( int y = sy; y <= ey; ++ y )
	{
		for ( int x = sx; x <= ex; ++ x )
		{
			Tile& t = mMap->GetTileAt( x, y );
			if ( !t.mRevealed )
				continue;

			if ( t.mType != Tile::Tile_NONE &&
			   ( t.mType < Tile::Tile_WALL ||
				 t.GetUsageId() == Tile::Tile_DOOR_FRAME ) )
			{
				if ( mMap->DebugSectors )
					window->SetDrawColor( mMap->GetColorForSectorId( t.mSectorId ) );
				else
					window->SetDrawColor( Color::GREY );
				window->DrawQuad2D( (x-mCenterX+cx) * mCellSize, (y-mCenterY+cy) * mCellSize, mCellSize, mCellSize );
			}
			
			if ( t.GetUsageId() == Tile::Tile_DOOR_FRAME )
			{
				if ( t.mLocked && !mMap->DebugSectors )
					window->SetDrawColor( mMap->GetColorForSectorId( t.mSectorId ) );
				else
					window->SetDrawColor( Color::DARK_GREY );

				if ( t.mType == Tile::Tile_DOOR_NORTH )
				{
					window->DrawQuad2D( (x-mCenterX+cx) * mCellSize, (y-mCenterY+cy) * mCellSize, mCellSize, mCellSize / 2 );
				}
				else if ( t.mType == Tile::Tile_DOOR_SOUTH )
				{
					window->DrawQuad2D( (x-mCenterX+cx) * mCellSize, (y-mCenterY+cy) * mCellSize + mCellSize / 2, mCellSize, mCellSize / 2 );
				}
				else if ( t.mType == Tile::Tile_DOOR_EAST )
				{
					window->DrawQuad2D( (x-mCenterX+cx) * mCellSize, (y-mCenterY+cy) * mCellSize, mCellSize / 2, mCellSize );
				}
				else if ( t.mType == Tile::Tile_DOOR_WEST )
				{
					window->DrawQuad2D( (x-mCenterX+cx) * mCellSize + mCellSize / 2, (y-mCenterY+cy) * mCellSize, mCellSize / 2, mCellSize );
				}
			}
			else if ( t.mType == Tile::Tile_EXIT )
			{
				window->SetDrawColor( Color::GREEN );
				window->DrawQuad2D( (x-mCenterX+cx) * mCellSize, (y-mCenterY+cy) * mCellSize, mCellSize, mCellSize );
			}
			else if ( t.mType == Tile::Tile_ENTRANCE )
			{
				window->SetDrawColor( Color::RED );
				window->DrawQuad2D( (x-mCenterX+cx) * mCellSize, (y-mCenterY+cy) * mCellSize, mCellSize, mCellSize );
			}
		}
	}
	
	float playerSize = mCellSize * 0.75f;
	window->SetDrawColor( Color::BLACK );
	window->DrawTriangle2D( ( mWidth + playerSize ) / 2.0f, ( mHeight + playerSize ) / 2.0f, playerSize * 1.4f, playerSize * 1.4f, mRotation );
	window->SetDrawColor( Color::GREEN );
	window->DrawTriangle2D( ( mWidth + playerSize ) / 2.0f, ( mHeight + playerSize ) / 2.0f, playerSize, playerSize, mRotation );
	window->DisableScissor();
	
	window->SetPerspective();
}
//---------------------------------------
void Minimap::UpdateVisibility( float range )
{
	std::vector< Tile* > tilesToCheck;
	PlotCircle( (int) mCenterX, (int) mCenterY, (int) ( range / mCellSize ), [&]( int x, int y ) -> bool
	{
		Tile& t = mMap->GetTileAt( x, y );
		tilesToCheck.push_back( &t );
		return true;
	}, false );

	for ( auto itr = tilesToCheck.begin(); itr != tilesToCheck.end(); ++itr )
	{
		Tile& t = **itr;

		PlotLine( (int) mCenterX, (int) mCenterY, t.x, t.y, [&]( int x, int y ) -> bool
		{
			Tile& tileToCheck = mMap->GetTileAt( x, y );

			tileToCheck.mRevealed = true;

			if ( tileToCheck.GetUsageId() == Tile::Tile_NONE ||
				 tileToCheck.GetUsageId() == Tile::Tile_DOOR_FRAME ||
				 tileToCheck.GetUsageId() == Tile::Tile_WALL || 
				 tileToCheck.GetUsageId() == Tile::Tile_WALL_CORNER || 
				 tileToCheck.GetUsageId() == Tile::Tile_WALL_CORNER_OUTSIDE )
					return false;
			return true;
		} );
	}
}
//---------------------------------------
void Minimap::RevealAll()
{
	for ( int y = 0; y < mMap->GetHeight(); ++y )
	{
		for ( int x = 0; x < mMap->GetWidth(); ++x )	
		{
			Tile& tile = mMap->GetTileAt( x, y );
			tile.mRevealed = true;
		}
	}
}
//---------------------------------------