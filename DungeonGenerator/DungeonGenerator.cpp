#include "DungeonGenerator.h"
#include "Util.h"
#include "Texture.h"
#include "Window.h"
#include "Mesh.h"
#include "StringUtil.h"
#include "Game.h"
#include "Door.h"
#include "Key.h"
#include "MapTile.h"
#include "MapObject.h"
#include "Logger.h"
#include "Enemy.h"

#include <stack>


Tile Tile::NULL_TILE;

//---------------------------------------
// Rules
Rule* Rule::CreateRule( const XmlReader::XmlReaderIterator& xmlItr )
{
	std::string type = xmlItr.GetAttributeAsString( "type" );
	Rule* rule = 0;

	if ( type == "random" )
	{
		rule = new Rule_Random( xmlItr );
	}
	else if ( type == "canSpawnOn" )
	{
		rule = new Rule_CanSpawnOn( xmlItr );
	}
	else if ( type == "canNotSpawnOn" )
	{
		rule = new Rule_CanNotSpawnOn( xmlItr );
	}
	else if ( type == "maxCount" )
	{
		rule = new Rule_MaxCount( xmlItr );
	}
	else if ( type == "notAdjacentToUsage" )
	{
		rule = new Rule_NotAdjacentToUsage( xmlItr );
	}
	else if ( type == "notAdjacentToObject" )
	{
		rule = new Rule_NotAdjacentToObject( xmlItr );
	}
	else if ( type == "notAdjacentToStyle" )
	{
		rule = new Rule_NotAdjacentToStyle( xmlItr );
	}
	else if ( type == "adjacentToUsage" )
	{
		rule = new Rule_AdjacentToUsage( xmlItr );
	}
	else if ( type == "adjacentToObject" )
	{
		rule = new Rule_AdjacentToObject( xmlItr );
	}
	else if ( type == "adjacentToStyle" )
	{
		rule = new Rule_AdjacentToStyle( xmlItr );
	}
	else if ( type == "distanceToUsage" )
	{
		rule = new Rule_DistanceToUsage( xmlItr );
	}
	else if ( type == "distanceToObject" )
	{
		rule = new Rule_DistanceToObject( xmlItr );
	}
	else if ( type == "distanceToStyle" )
	{
		rule = new Rule_DistanceToStyle( xmlItr );
	}
	else if ( type == "roomDoesHaveUsage" )
	{
		rule = new Rule_RoomDoesHaveUsage( xmlItr );
	}
	else if ( type == "roomDoesHaveStyle" )
	{
		rule = new Rule_RoomDoesHaveStyle( xmlItr );
	}
	else if ( type == "roomDoesHaveObject" )
	{
		rule = new Rule_RoomDoesHaveObject( xmlItr );
	}
	else if ( type == "roomDoesNotHaveUsage" )
	{
		rule = new Rule_RoomDoesNotHaveUsage( xmlItr );
	}
	else if ( type == "roomDoesNotHaveStyle" )
	{
		rule = new Rule_RoomDoesNotHaveStyle( xmlItr );
	}
	else if ( type == "roomDoesNotHaveObject" )
	{
		rule = new Rule_RoomDoesNotHaveObject( xmlItr );
	}
	else if ( type == "validDepths" )
	{
		rule = new Rule_ValidDepths( xmlItr );
	}
	else
	{
		DebugPrintf( "Unknown rule '%s'\n", type.c_str() );
	}

	assert( rule );
	return rule;
}
//---------------------------------------
// Rule_Random
Rule_Random::Rule_Random( const XmlReader::XmlReaderIterator& xmlItr )
{
	mPercentToBeTrue = xmlItr.GetAttributeAsInt( "percentToBeTrue" ) / 100.0f;
}
//---------------------------------------
bool Rule_Random::IsValid( Tile* tile )
{
	const float r = RNG::RandomUnit();
	return mPercentToBeTrue > 0 && r <= mPercentToBeTrue ? true : false;
}
//---------------------------------------
// Rule_UsageRule
Rule_UsageRule::Rule_UsageRule( const XmlReader::XmlReaderIterator& xmlItr )
{
	std::vector< std::string > usageStrings;
	xmlItr.GetAttributeAsCSV( "usages", usageStrings );
	for ( auto itr = usageStrings.begin(); itr != usageStrings.end(); ++itr )
	{
		mUsages.push_back( TileStyle::GetUsageIdFromString( *itr ) );
	}
}
//---------------------------------------
// Rule_ObjectRule
Rule_ObjectRule::Rule_ObjectRule( const XmlReader::XmlReaderIterator& xmlItr )
{
	xmlItr.GetAttributeAsCSV( "objectNames", mObjectNames );
}
//---------------------------------------
// Rule_StyleRule
Rule_StyleRule::Rule_StyleRule( const XmlReader::XmlReaderIterator& xmlItr )
{
	xmlItr.GetAttributeAsCSV( "styleNames", mStyleNames );
}
//---------------------------------------
// Rule_SpawnOn
Rule_SpawnOn::Rule_SpawnOn( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_UsageRule( xmlItr )
{}
//---------------------------------------
// Rule_CanSpawnOn
Rule_CanSpawnOn::Rule_CanSpawnOn( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_SpawnOn( xmlItr )
{}
//---------------------------------------
bool Rule_CanSpawnOn::IsValid( Tile* tile )
{
	for ( auto itr = mUsages.begin(); itr != mUsages.end(); ++itr )
	{
		if ( tile->GetUsageId() == *itr )
			return true;
	}
	return false;
}
//---------------------------------------
// Rule_CanNotSpawnOn
Rule_CanNotSpawnOn::Rule_CanNotSpawnOn( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_SpawnOn( xmlItr )
{}
//---------------------------------------
bool Rule_CanNotSpawnOn::IsValid( Tile* tile )
{
	for ( auto itr = mUsages.begin(); itr != mUsages.end(); ++itr )
	{
		if ( tile->GetUsageId() == *itr )
			return false;
	}
	return true;
}
//---------------------------------------
// Rule_MaxCount
Rule_MaxCount::Rule_MaxCount( const XmlReader::XmlReaderIterator& xmlItr )
{
	mMaxCount = xmlItr.GetAttributeAsInt( "count" );
	mCurrentCount = mMaxCount;
}
//---------------------------------------
bool Rule_MaxCount::IsValid( Tile* tile )
{
	return mCurrentCount > 0;
}
//---------------------------------------
void Rule_MaxCount::Reset()
{
	mCurrentCount = mMaxCount;
}
//---------------------------------------
// Rule_AdjacentTo
Rule_AdjacentTo::Rule_AdjacentTo( const XmlReader::XmlReaderIterator& xmlItr )
{
	mMargin = xmlItr.GetAttributeAsInt( "margin", 1 );
	std::vector< std::string > directionStrings;
	xmlItr.GetAttributeAsCSV( "directionsToCheck", directionStrings, "n,s,e,w" );
	memset( mDirectionsToCheck, 0, sizeof( bool ) * AD_COUNT );
	for ( auto itr = directionStrings.begin(); itr != directionStrings.end(); ++itr )
	{
		if ( *itr == "n" )
			mDirectionsToCheck[ AD_N ] = true;
		else if ( *itr == "s" )
			mDirectionsToCheck[ AD_S ] = true;
		else if ( *itr == "e" )
			mDirectionsToCheck[ AD_E ] = true;
		else if ( *itr == "w" )
			mDirectionsToCheck[ AD_W ] = true;
		else if ( *itr == "ne" )
			mDirectionsToCheck[ AD_NE ] = true;
		else if ( *itr == "nw" )
			mDirectionsToCheck[ AD_NW ] = true;
		else if ( *itr == "se" )
			mDirectionsToCheck[ AD_SE ] = true;
		else if ( *itr == "sw" )
			mDirectionsToCheck[ AD_SW ] = true;
		else
			DebugPrintf( "Invalid Adjacentcy Direction '%s'\n", itr->c_str() );
	}
}
//---------------------------------------
void Rule_AdjacentTo::GetTilesToCheck( Tile* tile, std::vector< Tile* >& tilesToCheck )
{
	if ( mDirectionsToCheck[ AD_N ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x, tile->y - i ) );
	if ( mDirectionsToCheck[ AD_S ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x, tile->y + i ) );
	if ( mDirectionsToCheck[ AD_E ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x - i, tile->y ) );
	if ( mDirectionsToCheck[ AD_W ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x + i, tile->y ) );
	if ( mDirectionsToCheck[ AD_NE ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x - i, tile->y - i ) );
	if ( mDirectionsToCheck[ AD_NW ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x + i, tile->y - i ) );
	if ( mDirectionsToCheck[ AD_SE ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x - i, tile->y + i ) );
	if ( mDirectionsToCheck[ AD_SW ] )
		for ( int i = 1; i <= mMargin; ++i )
			tilesToCheck.push_back( &mGrid->GetTileAt( tile->x + i, tile->y + i ) );
}
//---------------------------------------
// Rule_NotAdjacentToUsage
Rule_NotAdjacentToUsage::Rule_NotAdjacentToUsage( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_AdjacentTo( xmlItr )
	, Rule_UsageRule( xmlItr )
{
	mPassValue = false;
}
//---------------------------------------
bool Rule_NotAdjacentToUsage::IsValid( Tile* tile )
{
	std::vector< Tile* > tilesToCheck;
	GetTilesToCheck( tile, tilesToCheck );

	for ( auto itr = mUsages.begin(); itr != mUsages.end(); ++itr )
	{
		for ( auto jtr = tilesToCheck.begin(); jtr != tilesToCheck.end(); ++jtr )
		{
			if ( CheckTile( *jtr, *itr ) )
				return mPassValue;
		}
	}
	return !mPassValue;
}
//---------------------------------------
bool Rule_NotAdjacentToUsage::CheckTile( Tile* tile, int usage )
{
	return tile->GetUsageId() == usage;
}
//---------------------------------------
// Rule_NotAdjacentToObject
Rule_NotAdjacentToObject::Rule_NotAdjacentToObject( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_AdjacentTo( xmlItr )
	, Rule_ObjectRule( xmlItr )
{
	mPassValue = false;
}
//---------------------------------------
bool Rule_NotAdjacentToObject::IsValid( Tile* tile )
{
	std::vector< Tile* > tilesToCheck;
	GetTilesToCheck( tile, tilesToCheck );

	for ( auto itr = mObjectNames.begin(); itr != mObjectNames.end(); ++itr )
	{
		for ( auto jtr = tilesToCheck.begin(); jtr != tilesToCheck.end(); ++jtr )
		{
			if ( CheckTile( *jtr, *itr ) )
				return mPassValue;
		}
	}
	return !mPassValue;
}
//---------------------------------------
bool Rule_NotAdjacentToObject::CheckTile( Tile* tile, const std::string& objectName )
{
	return tile->HasObjectOfName( objectName );
}
//---------------------------------------
// Rule_NotAdjacentToStyle
Rule_NotAdjacentToStyle::Rule_NotAdjacentToStyle( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_AdjacentTo( xmlItr )
	, Rule_StyleRule( xmlItr )
{
	mPassValue = false;
}
//---------------------------------------
bool Rule_NotAdjacentToStyle::IsValid( Tile* tile )
{
	std::vector< Tile* > tilesToCheck;
	GetTilesToCheck( tile, tilesToCheck );

	for ( auto itr = mStyleNames.begin(); itr != mStyleNames.end(); ++itr )
	{
		for ( auto jtr = tilesToCheck.begin(); jtr != tilesToCheck.end(); ++jtr )
		{
			if ( CheckTile( *jtr, *itr ) )
				return mPassValue;
		}
	}
	return !mPassValue;
}
//---------------------------------------
bool Rule_NotAdjacentToStyle::CheckTile( Tile* tile, const std::string& styleName )
{
	return tile->HasStyleOfName( styleName );
}
//---------------------------------------
// Rule_AdjacentToUsage
Rule_AdjacentToUsage::Rule_AdjacentToUsage( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_NotAdjacentToUsage( xmlItr )
{
	mPassValue = true;
}
//---------------------------------------
// Rule_AdjacentToObject
Rule_AdjacentToObject::Rule_AdjacentToObject( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_NotAdjacentToObject( xmlItr )
{
	mPassValue = true;
}
//---------------------------------------
// Rule_AdjacentToStyle
Rule_AdjacentToStyle::Rule_AdjacentToStyle( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_NotAdjacentToStyle( xmlItr )
{
	mPassValue = true;
}
//---------------------------------------
// Rule_DistanceTo
Rule_DistanceTo::Rule_DistanceTo( const XmlReader::XmlReaderIterator& xmlItr )
{
	mMinDist = xmlItr.GetAttributeAsFloat( "minDistance", 0.0f );
	mMaxDist = xmlItr.GetAttributeAsFloat( "maxDistance", 0.0f );
}
//---------------------------------------
float Rule_DistanceTo::GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const
{
	return (float) GetManhattanDistance( startTile.x, startTile.y, endTile.x, endTile.y );
}
//---------------------------------------
bool Rule_DistanceTo::IsValid( Tile* tile )
{
	if ( tile->GetUsageId() == Tile::Tile_NONE )
		return false;

	int startX = tile->mRoom->x;
	int endX = startX + tile->mRoom->GetWidth();
	int startY = tile->mRoom->y;
	int endY = startY + tile->mRoom->GetHeight();
	bool found = false;

	for ( int y = startY; y < endY; ++y )
	{
		for ( int x = startX; x < endX; ++ x )
		{
			Tile& t = mGrid->GetTileAt( x, y );
			bool doCheck = false;

			// See if the tile should be checked
			doCheck = ShouldCheck( t );

			// Check tile against distance constraint
			if ( doCheck )
			{
				found = true;
				float d = GetDistanceBetween( *tile, t );
				bool valid = ( d >= mMinDist ) && ( mMaxDist == 0 || d <= mMaxDist );
				if ( !valid )
					return false;
			}
		}
	}
	return found;
}
//---------------------------------------
// Rule_DistanceToUsage
Rule_DistanceToUsage::Rule_DistanceToUsage( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_DistanceTo( xmlItr )
	, Rule_UsageRule( xmlItr )
{}
//---------------------------------------
bool Rule_DistanceToUsage::ShouldCheck( const Tile& tile ) const
{
	for ( auto itr = mUsages.begin(); itr != mUsages.end(); ++itr )
	{
		if ( tile.GetUsageId() == *itr )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
float Rule_DistanceToUsage::GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const
{
	return Rule_DistanceTo::GetDistanceBetween( startTile, endTile );
}
//---------------------------------------
// Rule_DistanceToObject
Rule_DistanceToObject::Rule_DistanceToObject( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_DistanceTo( xmlItr )
	, Rule_ObjectRule( xmlItr )
{}
//---------------------------------------
bool Rule_DistanceToObject::ShouldCheck( const Tile& tile ) const
{
	for ( auto itr = mObjectNames.begin(); itr != mObjectNames.end(); ++itr )
	{
		if ( tile.HasObjectOfName( *itr ) )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
float Rule_DistanceToObject::GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const
{
	return Rule_DistanceTo::GetDistanceBetween( startTile, endTile );
}
//---------------------------------------
// Rule_DistanceToStyle
Rule_DistanceToStyle::Rule_DistanceToStyle( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_DistanceTo( xmlItr )
	, Rule_StyleRule( xmlItr )
{}
//---------------------------------------
bool Rule_DistanceToStyle::ShouldCheck( const Tile& tile ) const
{
	for ( auto itr = mStyleNames.begin(); itr != mStyleNames.end(); ++itr )
	{
		if ( tile.HasStyleOfName( *itr ) )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
float Rule_DistanceToStyle::GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const
{
	return Rule_DistanceTo::GetDistanceBetween( startTile, endTile );
}
//---------------------------------------
// Rule_ValidDepths
Rule_ValidDepths::Rule_ValidDepths( const XmlReader::XmlReaderIterator& xmlItr )
{
	std::vector< std::string > validDepths;
	xmlItr.GetAttributeAsCSV( "validDepths", validDepths, "0" );
	for ( auto i = validDepths.begin(); i != validDepths.end(); ++i )
	{
		// Get string without leading or trailing spaces
		std::string depthStr = StringUtil::Strip( *i );

		int startDepth = -1;
		int endDepth = -1;
		for ( unsigned c = 0; c < depthStr.length(); ++c )
		{
			// Range marker
			if ( depthStr[c] == '-' )
			{
				if ( c == depthStr.length() - 1 )
				{
					DebugPrintf( "DepthValue syntax error: '%s'\n token '-' must appear before a number\n" );
					startDepth = -1;
					endDepth = -1;
					break;
				}
				// LessThan marker
				else if ( c == 0 )
				{
					startDepth = 0;
				}
				else
				{
					std::string endDepthStr;
					++c;
					while ( depthStr[c] == ' ' ) ++c;
					for ( ; StringUtil::IsNumeric( depthStr[c] ); ++c )
					{
						endDepthStr += depthStr[c];
					}
					--c;
					StringUtil::StringToType( endDepthStr, &endDepth );
				}
			}
			// GreaterThan marker
			else if ( depthStr[c] == '+' )
			{
				endDepth = 0;
			}
			// Number
			else if ( StringUtil::IsNumeric( depthStr[c] ) )
			{
				std::string endDepthStr;
				for ( ; StringUtil::IsNumeric( depthStr[c] ); ++c )
				{
					endDepthStr += depthStr[c];
				}
				--c;

				if ( endDepthStr == "0" )
				{
					startDepth = 0;
					endDepth = 0;
					break;
				}
				else if ( startDepth == 0 )
					StringUtil::StringToType( endDepthStr, &endDepth );
				else
					StringUtil::StringToType( endDepthStr, &startDepth );
			}
		}

		// DepthAll
		if ( startDepth == 0 && endDepth <= 0 )
		{
			AddValidDepth( new DepthValue_All() );
		}
		// DepthLessThan
		else if ( startDepth == 0 && endDepth > 0 )
		{
			AddValidDepth( new DepthValue_LessThan( endDepth ) );
		}
		// DepthGreaterThan
		else if ( startDepth > 0 && endDepth == 0 )
		{
			AddValidDepth( new DepthValue_GreaterThan( startDepth ) );
		}
		// DepthRange
		else if ( startDepth > 0 && endDepth > 0 )
		{
			AddValidDepth( new DepthValue_Range( startDepth, endDepth ) );
		}
		// DepthSingle
		else if ( startDepth > 0 && endDepth == -1 )
		{
			AddValidDepth( new DepthValue_Single( startDepth ) );
		}
	}
}
//---------------------------------------
Rule_ValidDepths::Rule_ValidDepths( const Rule_ValidDepths& other )
{
	for ( auto itr = other.mValidDepths.begin(); itr != other.mValidDepths.end(); ++itr )
		mValidDepths.push_back( (*itr)->Copy() );
	mGrid = other.mGrid;
}
//---------------------------------------
Rule_ValidDepths::~Rule_ValidDepths()
{
	for ( auto itr = mValidDepths.begin(); itr != mValidDepths.end(); ++itr )
		delete *itr;
}
//---------------------------------------
bool Rule_ValidDepths::IsValid( Tile* tile )
{
	return ValidateDepth( ( (DungeonGenerator*) mGrid )->GetCurrentDepth() );
}
//---------------------------------------
void Rule_ValidDepths::AddValidDepth( DepthValue* depth )
{
	mValidDepths.push_back( depth );
}
//---------------------------------------
bool Rule_ValidDepths::ValidateDepth( int currentDepth ) const
{
	for ( auto itr = mValidDepths.begin(); itr != mValidDepths.end(); ++itr )
	{
		if ( (*itr)->IsValidAtDepth( currentDepth ) )
			return true;
	}
	return false;
}
//---------------------------------------
// Rule_RoomContains
Rule_RoomContains::Rule_RoomContains()
	: Rule()
	, mPassValue( true )
{}
//---------------------------------------
bool Rule_RoomContains::IsValid( Tile* tile )
{
	Room* room = tile->mRoom;
	if ( room )
	{
		int startX = room->x;
		int endX = startX + room->GetWidth();
		int startY = room->y;
		int endY = startY + room->GetHeight();
		bool found = false;

		for ( int y = startY; y < endY; ++y )
		{
			for ( int x = startX; x < endX; ++ x )
			{
				Tile& t = mGrid->GetTileAt( x, y );

				if ( CheckTile( t ) )
					return mPassValue;
			}
		}
	}
	return !mPassValue;
}
//---------------------------------------
// Rule_RoomDoesNotHaveUsage
Rule_RoomDoesNotHaveUsage::Rule_RoomDoesNotHaveUsage( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_RoomContains()
	, Rule_UsageRule( xmlItr )
{
	mPassValue = false;
}
//---------------------------------------
bool Rule_RoomDoesNotHaveUsage::CheckTile( const Tile& tile )
{
	for ( auto itr = mUsages.begin(); itr != mUsages.end(); ++itr )
	{
		if ( tile.GetUsageId() == *itr )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
// Rule_RoomDoesHaveUsage
Rule_RoomDoesHaveUsage::Rule_RoomDoesHaveUsage( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_RoomDoesNotHaveUsage( xmlItr )
{
	mPassValue = true;
}
//---------------------------------------
// Rule_RoomDoesNotHaveStyle
Rule_RoomDoesNotHaveStyle::Rule_RoomDoesNotHaveStyle( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_RoomContains()
	, Rule_StyleRule( xmlItr )
{
	mPassValue = false;
}
//---------------------------------------
bool Rule_RoomDoesNotHaveStyle::CheckTile( const Tile& tile )
{
	for ( auto itr = mStyleNames.begin(); itr != mStyleNames.end(); ++itr )
	{
		if ( tile.HasStyleOfName( *itr ) )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
// Rule_RoomDoesHaveStyle
Rule_RoomDoesHaveStyle::Rule_RoomDoesHaveStyle( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_RoomDoesNotHaveStyle( xmlItr )
{
	mPassValue = true;
}
//---------------------------------------
// Rule_RoomDoesNotHaveObject
Rule_RoomDoesNotHaveObject::Rule_RoomDoesNotHaveObject( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_RoomContains()
	, Rule_ObjectRule( xmlItr )
{
	mPassValue = false;
}
//---------------------------------------
bool Rule_RoomDoesNotHaveObject::CheckTile( const Tile& tile )
{
	for ( auto itr = mObjectNames.begin(); itr != mObjectNames.end(); ++itr )
	{
		if ( tile.HasObjectOfName( *itr ) )
		{
			return true;
		}
	}
	return false;
}
//---------------------------------------
// Rule_RoomDoesHaveObject
Rule_RoomDoesHaveObject::Rule_RoomDoesHaveObject( const XmlReader::XmlReaderIterator& xmlItr )
	: Rule_RoomDoesNotHaveObject( xmlItr )
{
	mPassValue = true;
}
//---------------------------------------


//---------------------------------------
// RuledObject
RuledObject::~RuledObject()
{
	for ( auto itr = mRules.begin(); itr != mRules.end(); ++itr )
		delete *itr;
}
//---------------------------------------
void RuledObject::ResetRules()
{
	for ( auto itr = mRules.begin(); itr != mRules.end(); ++itr )
		(*itr)->Reset();
}
//---------------------------------------
void RuledObject::AddRule( Rule* rule )
{
	mRules.push_back( rule );
}
//---------------------------------------
void RuledObject::AddRules( const std::vector< Rule* >& rules )
{
	mRules.insert( mRules.end(), rules.begin(), rules.end() );
}
//---------------------------------------
bool RuledObject::ValidateRules( Tile* tile )
{
	bool ret = true;
	for ( auto itr = mRules.begin(); itr != mRules.end(); ++itr )
	{
		Rule* r = *itr;
		if ( r && !r->IsValid( tile ) )
		{
			ret = false;
			break;
		}
	}
	return ret;
}
//---------------------------------------
void RuledObject::NotifySuccess()
{
	for ( auto itr = mRules.begin(); itr != mRules.end(); ++itr )
	{
		(*itr)->NotifySuccess();
	}
}
//---------------------------------------
/*void RuledObject::LoadRulesFromXML( const XmlReader::XmlReaderIterator& xmlItr, TileGrid* ownerGrid )
{
	for ( XmlReader::XmlReaderIterator ruleItr = xmlItr.NextChild( "Rule" );
		ruleItr.IsValid(); ruleItr = ruleItr.NextSibling( "Rule" ) )
	{
		Rule* rule = Rule::CreateRule( ruleItr );
		rule->mGrid = ownerGrid;
		mRules.push_back( rule );
	}
}*/
//---------------------------------------
void RuledObject::CopyRulesFrom( RuledObject* obj )
{
	for ( auto itr = obj->mRules.begin(); itr != obj->mRules.end(); ++itr )
		mRules.push_back( (*itr)->Copy() );
	//mRules.insert( mRules.end(), obj->mRules.begin(), obj->mRules.end() );
}
//---------------------------------------


//---------------------------------------
// EventObject
void EventObject::LoadEventsFromXML( const XmlReader::XmlReaderIterator& xmlItr )
{
	for ( XmlReader::XmlReaderIterator itr = xmlItr.NextChild( "Signal" );
		  itr.IsValid(); itr = itr.NextSibling( "Signal" ) )
	{
		std::string type = itr.GetAttributeAsString( "type" );
		std::string tag = itr.GetAttributeAsString( "tag" );
		unsigned int taghash = EventListener::TagFromString( tag );
		int eventType = EventListener::SignalFromString( type );
		DebugPrintf( "Load signal: %s %s\n", type.c_str(), tag.c_str() );
		mSignals[ eventType ].push_back( taghash );
	}

	for ( XmlReader::XmlReaderIterator itr = xmlItr.NextChild( "Slot" );
		  itr.IsValid(); itr = itr.NextSibling( "Slot" ) )
	{
		std::string tag = itr.GetAttributeAsString( "tag" );
		std::string commands = itr.GetAttributeAsString( "commands" );
		DebugPrintf( "Load slot: %s %s\n", tag.c_str(), commands.c_str() );
		unsigned int taghash = EventListener::TagFromString( tag );
		mSlots[ taghash ].push_back( commands );
	}
}
//---------------------------------------
void EventObject::SetupEvents( Entity* entity ) const
{
	for ( auto itr = mSignals.begin(); itr != mSignals.end(); ++itr )
	{
		for ( auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr )
			entity->RegisterSignal( itr->first, *jtr );
	}

	for ( auto itr = mSlots.begin(); itr != mSlots.end(); ++itr )
	{
		for ( auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr )
			entity->RegisterSlot( itr->first, *jtr );
	}
}
//---------------------------------------


//---------------------------------------
// TileObject
int TileObject::GetUsageIdFromString( const std::string& str )
{
	if ( str == "static" )
		return Usage_STATIC;
	else if ( str == "dynamic" )
		return Usage_DYNAMIC;
	else if ( str == "pickup" )
		return Usage_PICKUP;
	else if ( str == "light" )
		return Usage_LIGHT;
	else if ( str == "enemy" )
		return Usage_ENEMY;
	else if ( str == "spawner" )
		return Usage_SPAWNER;

	// Fallback to static on bad type
	DebugPrintf( "TileObject: '%s' is not a valid usage\n", str.c_str() );
	return Usage_STATIC;
}
//---------------------------------------


//---------------------------------------
// SpawnList
const std::string& SpawnList::GetRandomObject() const
{
	return mList[ RNG::RandomIndex( mList.size() ) ];
}
//---------------------------------------


//---------------------------------------
// TileObject_Spawner
const std::string& TileObject_Spawner::GetObjectToSpawn() const
{
	return mList->GetRandomObject();
}
//---------------------------------------


//---------------------------------------
// TileStyle
int TileStyle::GetUsageIdFromString( const std::string& str )
{
	if ( str == "wall" )
		return Tile::Tile_WALL;
	else if ( str == "floor" )
		return Tile::Tile_FLOOR;
	else if ( str == "corner" )
		return Tile::Tile_WALL_CORNER;
	else if ( str == "corner_outside" )
		return Tile::Tile_WALL_CORNER_OUTSIDE;
	else if ( str == "stair_base" )
		return Tile::Tile_STAIR_BASE;
	else if ( str == "stair_top" )
		return Tile::Tile_STAIR_TOP;
	else if ( str == "door" )
		return Tile::Tile_DOOR;
	else if ( str == "door_frame" )
		return Tile::Tile_DOOR_FRAME;
	else if ( str == "exit" )
		return Tile::Tile_EXIT;
	else if ( str == "entrance" )
		return Tile::Tile_ENTRANCE;
	return Tile::Tile_NONE;
}
//---------------------------------------


//---------------------------------------
// Useable
void Useable::ResetRules()
{
	RuledObject::ResetRules();
//	mObject->ResetRules();
}
//---------------------------------------


//---------------------------------------
// RoomTemplate
RoomTemplate::RoomTemplate()
	: mMinRoomSizeX( -1 )
	, mMinRoomSizeY( -1 )
	, mMaxRoomSizeX( -1 )
	, mMaxRoomSizeY( -1 )
{}
//---------------------------------------
void RoomTemplate::CopyDataFrom( RoomTemplate* room )
{
	// Append arrays
	for ( auto itr = room->mStyles.begin(); itr != room->mStyles.end(); ++itr )
	{
		mStyles[ itr->first ].insert( mStyles[ itr->first ].end(), room->mStyles[ itr->first ].begin(), room->mStyles[ itr->first ].end() );
	}
	mObjects.insert( mObjects.end(), room->mObjects.begin(), room->mObjects.end() );
	mMinRoomSizeX = room->mMinRoomSizeX;
	mMinRoomSizeY = room->mMinRoomSizeY;
	mMaxRoomSizeX = room->mMaxRoomSizeX;
	mMaxRoomSizeY = room->mMaxRoomSizeY;
}
//---------------------------------------
TileStyle* RoomTemplate::GetStyle( Tile* tile )
{
	std::vector< Useable* >& styles = mStyles[ tile->GetUsageId() ];
	if ( !styles.empty() )
	{
		for ( auto itr = styles.begin(); itr != styles.end(); ++itr )
		{
			Useable* u = *itr;
			if ( u->ValidateRules( tile ) )
			{
				TileStyle* obj = (TileStyle*) u->mObject;
				if ( obj->ValidateRules( tile ) )
				{
					u->NotifySuccess();
					obj->NotifySuccess();
					return obj;
				}
			}
		}
	}
	return 0;
}
//---------------------------------------
void RoomTemplate::AddStyle( Useable* style )
{
	if ( style )
		mStyles[ ((TileStyle*)style->mObject)->mUsageId ].push_back( style );
}
//---------------------------------------
bool RoomTemplate::HasStyleForUsage( int usage ) const
{
	return mStyles.find( usage ) != mStyles.end();
}
//---------------------------------------
TileObject* RoomTemplate::GetObject( Tile* tile )
{
	for ( auto itr = mObjects.begin(); itr != mObjects.end(); ++itr )
	{
		Useable* u = *itr;
		if ( u->ValidateRules( tile ) )
		{
			TileObject* obj = (TileObject*) u->mObject;
			if ( obj->ValidateRules( tile ) )
			{
				u->NotifySuccess();
				obj->NotifySuccess();
				return obj;
			}
		}
	}

	return 0;
}
//---------------------------------------
void RoomTemplate::AddObject( Useable* object )
{
	if ( object )
		mObjects.push_back( object );
}
//---------------------------------------
void RoomTemplate::ResetRules()
{
	RuledObject::ResetRules();
	for ( auto itr = mObjects.begin(); itr != mObjects.end(); ++itr )
		(*itr)->ResetRules();
	for ( auto itr = mStyles.begin(); itr != mStyles.end(); ++itr )
		for ( auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr )
			(*jtr)->ResetRules();
}
//---------------------------------------
void RoomTemplate::ResetObjects()
{
	for ( auto itr = mObjects.begin(); itr != mObjects.end(); ++itr )
		(*itr)->mObject->ResetRules();
	for ( auto itr = mStyles.begin(); itr != mStyles.end(); ++itr )
		for ( auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr )
			(*jtr)->mObject->ResetRules();
}
//---------------------------------------
void RoomTemplate::SetMinSize( int x, int y )
{
	mMinRoomSizeX = x;
	mMinRoomSizeY = y;
}
//---------------------------------------
void RoomTemplate::SetMaxSize( int x, int y )
{
	mMaxRoomSizeX = x;
	mMaxRoomSizeY = y;
}
//---------------------------------------



//---------------------------------------
// Tile
int Tile::GetUsageId() const
{
	if ( mType == Tile_FLOOR )
		return Tile_FLOOR;
	else if ( mType == Tile_WALL_NORTH || mType == Tile_WALL_SOUTH || mType == Tile_WALL_EAST || mType ==  Tile_WALL_WEST )
		return Tile_WALL;
	else if ( mType == Tile_WALL_CORNER_NORTHWEST || mType == Tile_WALL_CORNER_NORTHEAST || mType == Tile_WALL_CORNER_SOUTHWEST || mType ==  Tile_WALL_CORNER_SOUTHEAST )
		return Tile_WALL_CORNER;
	else if ( mType == Tile_WALL_CORNER_OUTSIDE_NORTHWEST || mType == Tile_WALL_CORNER_OUTSIDE_NORTHEAST || mType == Tile_WALL_CORNER_OUTSIDE_SOUTHWEST || mType ==  Tile_WALL_CORNER_OUTSIDE_SOUTHEAST )
		return Tile_WALL_CORNER_OUTSIDE;
	else if ( mType == Tile_STAIR_BASE_NORTH || mType == Tile_STAIR_BASE_SOUTH || mType == Tile_STAIR_BASE_EAST || mType ==  Tile_STAIR_BASE_WEST  )
		return Tile_STAIR_BASE;
	else if ( mType == Tile_STAIR_TOP_NORTH || mType == Tile_STAIR_TOP_SOUTH || mType == Tile_STAIR_TOP_EAST || mType ==  Tile_STAIR_TOP_WEST  )
		return Tile_STAIR_TOP;
	else if ( mType == Tile_DOOR_EAST || mType == Tile_DOOR_NORTH || mType == Tile_DOOR_WEST || mType == Tile_DOOR_SOUTH )
		return Tile_DOOR_FRAME;
	else if ( mType == Tile_DOOR )
		return Tile_DOOR;
	else if ( mType == Tile_EXIT )
		return Tile_EXIT;
	else if ( mType == Tile_ENTRANCE )
		return Tile_ENTRANCE;
	return Tile_NONE;
}
//---------------------------------------
float Tile::GetOrientation() const
{
	if      ( mType == Tile_WALL_WEST ||
		      mType == Tile_WALL_CORNER_OUTSIDE_SOUTHWEST ||
		      mType == Tile_WALL_CORNER_SOUTHEAST ||
			  mType == Tile_STAIR_BASE_EAST ||
			  mType == Tile_STAIR_TOP_EAST ||
			  mType == Tile_DOOR_SOUTH )
		return 270.0f;
	else if ( mType == Tile_WALL_EAST ||
		      mType == Tile_WALL_CORNER_OUTSIDE_NORTHEAST ||
			  mType == Tile_WALL_CORNER_NORTHWEST ||
			  mType == Tile_STAIR_BASE_WEST ||
			  mType == Tile_STAIR_TOP_WEST ||
			  mType == Tile_DOOR_NORTH )
		return 90.0f;
	else if ( mType == Tile_WALL_SOUTH ||
		      mType == Tile_WALL_CORNER_OUTSIDE_SOUTHEAST ||
			  mType == Tile_WALL_CORNER_SOUTHWEST ||
			  mType == Tile_STAIR_BASE_NORTH || 
			  mType == Tile_STAIR_TOP_NORTH || 
			  mType == Tile_DOOR_EAST )
		return 180.0f;
	return 0;
}
//---------------------------------------
bool Tile::HasObjectOfName( const std::string& name ) const
{
	return mObject && mObject->mName == name;
}
//---------------------------------------
bool Tile::HasStyleOfName( const std::string& name ) const
{
	return mStyle && mStyle->mName == name;
}
//---------------------------------------
bool Tile::CanBeLocked() const
{
	return mStyle && mStyle->mCanBeLocked;
}
//---------------------------------------


//---------------------------------------
// DungeonGenerator
DungeonGenerator::DungeonGenerator()
	: mMaxRoomCount( 0 )
	, mMinRoomSizeX( 0 )
	, mMaxRoomSizeX( 0 )
	, mMinRoomSizeY( 0 )
	, mMaxRoomSizeY( 0 )
	, mVerticalChance( 0.55f )
	, mVerticalBiasUp( 0.5f )
	, mVerticalBiasDown( 0.5f )
	, mChanceToLock( 0.5f )
	, mDoorChance( 0.5f )
	, DebugSectors( false )
	, mCurrentDepth( 0 )
{
	mDirectionBias[0] = 0;
	mDirectionBias[1] = 0;
}
//---------------------------------------
DungeonGenerator::DungeonGenerator( int width, int height, int maxRoomCount, int minRoomWidth, int maxRoomWidth, int minRoomHeight, int maxRoomHeight )
	: TileGrid( width, height )
	, mMaxRoomCount( maxRoomCount )
	, mMinRoomSizeX( minRoomWidth )
	, mMaxRoomSizeX( maxRoomWidth )
	, mMinRoomSizeY( minRoomWidth )
	, mMaxRoomSizeY( maxRoomHeight )
	, mVerticalChance( 0.5f )
	, mVerticalBiasUp( 0.5f )
	, mVerticalBiasDown( 0.5f )
	, mChanceToLock( 0.5f )
	, mDoorChance( 0.5f )
	, DebugSectors( false )
	, mCurrentDepth( 0 )
{
	mDirectionBias[0] = 0;
	mDirectionBias[1] = 0;
}
//---------------------------------------
DungeonGenerator::~DungeonGenerator()
{}
//---------------------------------------
void DungeonGenerator::Init( int width, int height, int maxRoomCount, int minRoomWidth, int maxRoomWidth, int minRoomHeight, int maxRoomHeight )
{
	Clear();
	Resize( width, height );

	mMaxRoomCount = maxRoomCount;
	mMinRoomSizeX = minRoomWidth;
	mMaxRoomSizeX = maxRoomWidth;
	mMinRoomSizeY = minRoomWidth;
	mMaxRoomSizeY = maxRoomHeight;

	mCurrentDepth = 0;

	mDirectionBias[0] = 0;
	mDirectionBias[1] = 0;

	for ( auto itr = mRoomTemplates.begin(); itr != mRoomTemplates.end(); ++itr )
		delete *itr;
	mRoomTemplates.clear();
}
//---------------------------------------
void DungeonGenerator::Generate()
{
	Clear();

	mFloorName = mName + " Level " + StringUtil::ToString( ++mCurrentDepth );

	// Make a central room to start with
	mRooms.push_back( new Room() );
	GenerateRoom( *mRooms.back() );
	AddRoom( *mRooms.back(), mWidth / 2 - mRooms.back()->GetWidth() / 2, mHeight / 2 - mRooms.back()->GetHeight() / 2, 0 );

	// Make a new room and try to randomly connect it to an existing room
	// If the new room doesn't fit just throw it away and make another new room
	//
	// A possible improvement to this algorithm would be to figure out the available area
	// for a new room and use that to ensure the generated room would fit. The main advantage
	// of this approach would be reduced iterations which could potentially speed up the generation
	// process.
	const int n = mWidth * mHeight * 2;
	for ( int i = 0; i < n; ++i )
	{
		// Max of zero
		if ( mMaxRoomCount != 0 )
		{
			// Check if we made enough rooms
			if ( mRooms.size() == mMaxRoomCount )
				break;
		}

		// Get a place to put a door
		int doorX, doorY, dir;
		GetDoorLocation( doorX, doorY );
		dir = GetDoorDirection( doorX, doorY );

		if ( dir != Dir_INVALID )
		{
			int rx, ry;

			Room* newRoom = new Room();
			GenerateRoom( *newRoom );

			if ( dir == Dir_NORTH )
			{
				rx = doorX - newRoom->GetWidth() / 2;
				ry = doorY - newRoom->GetHeight();
			}
			else if ( dir == Dir_SOUTH )
			{
				rx = doorX - newRoom->GetWidth() / 2;
				ry = doorY + 1;
			}
			else if ( dir == Dir_EAST )
			{
				rx = doorX - newRoom->GetWidth();
				ry = doorY - newRoom->GetHeight() / 2;
			}
			else if ( dir == Dir_WEST )
			{
				rx = doorX + 1;
				ry = doorY - newRoom->GetHeight() / 2;
			}

			if ( CanRoomFitHere( rx, ry, newRoom->GetWidth(), newRoom->GetHeight(), newRoom->GetTileAt( 0, 0 ).z ) )
			{
				float z = GetTileAt( doorX, doorY ).z;
				float r = (float) RNG::RandomUnit();
				if ( r <= mVerticalChance )
				{
					float up = mVerticalBiasUp - r;
					float down = mVerticalBiasDown - r;
					if ( up > down && mVerticalBiasUp != 0.0f )
						z += 1.0f;
					else if ( mVerticalBiasDown != 0.0f )
						z -= 1.0f;
				}
				AddRoom( *newRoom, rx, ry, z );
				ConnectRooms( doorX, doorY, dir );
				mRooms.push_back( newRoom );
			}
			else
			{
				delete newRoom;
			}
		}
	}

	GatherDoors();

	// Find a starting location
	PlaceEntrance();
	
	// Figure out a critical path through the level
	CalculateSectors();

	// Find an ending location
	PlaceExit();

	// Generate objects and world geometry to spawn
	GenerateSpawnData();

	// Clear temp cached data
	mDoors.clear();
	mSectorsToVisit.clear();
	mOrderOfVisitation.clear();
}
//---------------------------------------
std::string DungeonGenerator::ToText()
{
	std::string level = "Ledgend:\n ' '\tEmpty\n '.'\tFloor\n '#'\tWall\n";
	for ( int y = 0; y < mHeight; ++y )
	{
		for ( int x = 0; x < mWidth; ++x )	
		{
			const int c = GetTileAt( x, y ).mType;

			if ( c == Tile::Tile_NONE )
				level += ' ';
			else if ( c == Tile::Tile_FLOOR )
				level += '.';
			else if ( c == Tile::Tile_WALL_CORNER )
				level += '#';
			else if ( c == Tile::Tile_WALL_EAST )
				level += '|';
			else if ( c == Tile::Tile_WALL_WEST )
				level += '|';
			else if ( c == Tile::Tile_WALL_SOUTH )
				level += '-';
			else if ( c == Tile::Tile_WALL_NORTH )
				level += '-';
			else if ( c == Tile::Tile_EXIT )
				level += 'X';
			else if ( c == Tile::Tile_ENTRANCE )
				level += 'E';
			else
				level += '?';
		}
		level += '\n';
	}
	return level;
}
//---------------------------------------
void DungeonGenerator::GenerateRoom( Room& room )
{
	RoomTemplate* tmpl = 0;

//	std::random_shuffle( mRoomTemplates.begin(), mRoomTemplates.end() );
	for ( auto itr = mRoomTemplates.begin(); itr != mRoomTemplates.end(); ++itr )
	{
		RoomTemplate* roomTmpl = *itr;
		if ( roomTmpl->ValidateRules( &Tile::NULL_TILE ) )
		{
			tmpl = roomTmpl;
			break;
		}
	}

	// In case no templates were specified/valid
	if ( !tmpl )
	{
		DebugPrintf( "Failed to generate room: No valid rooms found at depth %d\n", mCurrentDepth );
		tmpl = &mDummyRoomTmpl;
	}

	int minSizeX = mMinRoomSizeX;
	int maxSizeX = mMaxRoomSizeX;
	int minSizeY = mMinRoomSizeY;
	int maxSizeY = mMaxRoomSizeY;

	if ( tmpl->mMinRoomSizeX > 0 )
		minSizeX = tmpl->mMinRoomSizeX;
	if ( tmpl->mMaxRoomSizeX > 0 )
		maxSizeX = tmpl->mMaxRoomSizeX;
	if ( tmpl->mMinRoomSizeY > 0 )
		minSizeY = tmpl->mMinRoomSizeY;
	if ( tmpl->mMaxRoomSizeY > 0 )
		maxSizeY = tmpl->mMaxRoomSizeY;

	int w = RNG::RandomInRange( minSizeX, maxSizeX );
	int h = RNG::RandomInRange( minSizeY, maxSizeY );
	room.Resize( w, h );
	room.mTemplate = tmpl;

	// 1) fill room
	for ( int y = 0; y < h; ++ y )
	{
		for ( int x = 0; x < w; ++x )
		{
			Tile* tile = &room.GetTileAt( x, y );
			tile->mRoomTemplate = tmpl;

			if ( x == 0 && y == 0 )
			{
				tile->mType = Tile::Tile_WALL_CORNER_SOUTHEAST;
			}
			else if ( x == 0 && y == h - 1 )
			{
				tile->mType = Tile::Tile_WALL_CORNER_NORTHEAST;
			}
			else if ( x == w - 1 && y == h - 1 )
			{
				tile->mType = Tile::Tile_WALL_CORNER_NORTHWEST;
			}
			else if ( x == w - 1 && y == 0 )
			{
				tile->mType = Tile::Tile_WALL_CORNER_SOUTHWEST;
			}
			else if ( x == 0 )
			{
				tile->mType = Tile::Tile_WALL_WEST;
			}
			else if ( x == w - 1 )
			{
				tile->mType = Tile::Tile_WALL_EAST;
			}
			else if ( y == 0 )
			{
				tile->mType = Tile::Tile_WALL_SOUTH;
			}
			else if ( y == h - 1 )
			{
				tile->mType = Tile::Tile_WALL_NORTH;
			}
			else
			{
				tile->mType = Tile::Tile_FLOOR;
			}
		}
	}
}
//---------------------------------------
void DungeonGenerator::AddRoom( Room& room, int x, int y, float z )
{
	room.x = x;
	room.y = y;
	room.mTemplate->NotifySuccess();

	int rx = 0;
	int ry = 0;
	for ( int gy = y; gy < y + room.GetHeight(); ++gy )
	{
		for ( int gx = x; gx < x + room.GetWidth(); ++gx )
		{
			SetTileAt( gx, gy, room.GetTileAt( rx, ry ) );
			GetTileAt( gx, gy ).z = z;
			GetTileAt( gx, gy ).mRoom = &room;
			++rx;
		}
		rx = 0;
		++ry;
	}
}
//---------------------------------------
void DungeonGenerator::Clear()
{
	// Fill map with empty tiles
	for ( int y = 0; y < mHeight; ++y )
	{
		for ( int x = 0; x < mWidth; ++x )	
		{
			mTiles[ x * mHeight + y ] = Tile();
		}
	}

	mSectorColors.clear();
	mTilesBySector.clear();

	for ( auto itr = mRoomTemplates.begin(); itr != mRoomTemplates.end(); ++itr )
	{
		RoomTemplate* tmpl = *itr;
		tmpl->ResetRules();
		tmpl->ResetObjects();
	}

	for ( auto itr = mRooms.begin(); itr != mRooms.end(); ++itr )
		delete *itr;
	mRooms.clear();
}
//---------------------------------------
void DungeonGenerator::PlaceEntrance()
{
	const Room* entranceRoom = 0;
	std::vector< const Room* > possibleStarts;

	// Get rooms that can be the start
	for ( auto itr = mRooms.begin(); itr != mRooms.end(); ++itr )
	{
		const Room& room = **itr;
		if ( room.mTemplate->HasStyleForUsage( Tile::Tile_ENTRANCE ) )
		{
			possibleStarts.push_back( &room );
		}
	}

	if ( possibleStarts.empty() )
	{
		WarnFail( "Failed to find player spawn - ensure at least one room has 'entrance' style\n" );
	}
	else
	{
		std::random_shuffle( possibleStarts.begin(), possibleStarts.end() );
		entranceRoom = possibleStarts.front();

		glm::vec3 pos( entranceRoom->x + entranceRoom->GetWidth() / 2.0f, 1.0f, entranceRoom->y + entranceRoom->GetHeight() / 2.0f );
		Tile& t = GetTileAt( (int) pos.x, (int) pos.z );
		t.mType = Tile::Tile_ENTRANCE;
		pos.y = t.z;
		mEntranceLocation = pos;
	}
}
//---------------------------------------
void DungeonGenerator::PlaceExit()
{
	bool placedExit = false;
	for ( auto itr = mOrderOfVisitation.rbegin(); itr != mOrderOfVisitation.rend(); ++itr )
	{
		const int exitSector = *itr;
		const Room* exitRoom = GetRoomBySectorId( exitSector );
		if ( exitRoom && exitRoom->mTemplate->HasStyleForUsage( Tile::Tile_EXIT ) )
		{
			Tile& t = *mTilesBySector[ exitSector ][ RNG::RandomIndex( mTilesBySector[ exitSector ].size() ) ];
			t.mType = Tile::Tile_EXIT;
			mExitLocation = glm::vec3( t.x, t.z, t.y );
			placedExit = true;
			break;
		}
	}

	if ( !placedExit )
	{
		WarnFail( "Failed to place level exit - ensure at least one room has 'exit' style\n" );

		const int exitSector = mOrderOfVisitation.front();
		const Room* exitRoom = GetRoomBySectorId( exitSector );
		Tile& t = *mTilesBySector[ exitSector ][ RNG::RandomIndex( mTilesBySector[ exitSector ].size() ) ];
		t.mType = Tile::Tile_EXIT;
		mExitLocation = glm::vec3( t.x, t.z, t.y );
	}
}
//---------------------------------------
void DungeonGenerator::GetDoorLocation( int& x, int& y )
{
	WeightedRandom< int > randRoom;
	for ( size_t i = 0; i < mRooms.size(); ++i )
	{
		float weight = abs( mRooms[i]->x ) * mDirectionBias[0] + abs( mRooms[i]->y ) * mDirectionBias[1];
		randRoom.Add( i, weight );
	}

	int index;
	if ( randRoom.GetTotalWeight() == 0 )
		index = RNG::RandomInRange( 0U, mRooms.size() - 1 );
	else
		index = randRoom.Evaluate();
	const Room& room = *mRooms[ index ];
	const int n = room.GetWidth() * room.GetHeight();
	for ( int i = 0; i < n; ++i )
	{
		x = RNG::RandomInRange( room.x, room.x + room.GetWidth() );
		y = RNG::RandomInRange( room.y, room.y + room.GetHeight() );
		
		if ( GetTileAt( x, y ).mType > Tile::Tile_WALL )
			break;
	}
}
//---------------------------------------
int DungeonGenerator::GetDoorDirection( int x, int y )
{
	int dir = Dir_INVALID;
	const int tileType = GetTileAt( x, y ).mType;
	if ( tileType == Tile::Tile_WALL_EAST )
		dir = Dir_WEST;
	else if ( tileType == Tile::Tile_WALL_WEST )
		dir = Dir_EAST;
	else if ( tileType == Tile::Tile_WALL_NORTH )
		dir = Dir_SOUTH;
	else if ( tileType == Tile::Tile_WALL_SOUTH )
		dir = Dir_NORTH;
	return dir;
}
//---------------------------------------
bool DungeonGenerator::CanRoomFitHere( int px, int py, int w, int h, float z )
{
	for ( int y = py; y < py + h; ++y )
	{
		for ( int x = px; x < px + w; ++x )
		{
			if ( x < 0 || x >= mWidth || y < 0 || y >= mHeight )
				return false;
			if ( /*GetTileAt( x, y ).z == z &&*/ GetTileAt( x, y ).mType != Tile::Tile_NONE )
				return false;
		}
	}
	return true;
}
//---------------------------------------
void DungeonGenerator::ConnectRooms( int doorX, int doorY, int dir )
{
	SetTileAt( doorX, doorY, Tile::Tile_FLOOR );

	if ( dir == Dir_NORTH )
	{
		if ( GetTileAt( doorX, doorY ).z < GetTileAt( doorX, doorY - 1 ).z )
		{
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_BASE_NORTH );
			SetTileAt( doorX, doorY - 1, Tile::Tile_STAIR_TOP_NORTH );
		}
		else if ( GetTileAt( doorX, doorY ).z > GetTileAt( doorX, doorY - 1 ).z )
		{
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_TOP_SOUTH  );
			SetTileAt( doorX, doorY - 1, Tile::Tile_STAIR_BASE_SOUTH );
		}
		else
		{
			// Doors
			if ( GetTileAt( doorX, doorY - 1 ).mRoomTemplate->HasStyleForUsage( Tile::Tile_DOOR ) && RandomPercentCheck( mDoorChance ) )
			{
				SetTileAt( doorX, doorY - 1, Tile::Tile_DOOR_NORTH );
			}
			else
				SetTileAt( doorX, doorY - 1, Tile::Tile_FLOOR );
		}

		if ( GetTileAt( doorX - 1, doorY - 1 ).mType == Tile::Tile_WALL_NORTH )
			SetTileAt( doorX - 1, doorY - 1, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHWEST );
		else
			SetTileAt( doorX - 1, doorY - 1, Tile::Tile_WALL_WEST );

		if ( GetTileAt( doorX + 1, doorY - 1 ).mType == Tile::Tile_WALL_NORTH )
			SetTileAt( doorX + 1, doorY - 1, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHEAST );
		else
			SetTileAt( doorX + 1, doorY - 1, Tile::Tile_WALL_EAST );

		if ( GetTileAt( doorX - 1, doorY ).mType == Tile::Tile_WALL_SOUTH )
			SetTileAt( doorX - 1, doorY, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHWEST );
		else
			SetTileAt( doorX - 1, doorY, Tile::Tile_WALL_WEST );

		if ( GetTileAt( doorX + 1, doorY ).mType == Tile::Tile_WALL_SOUTH )
			SetTileAt( doorX + 1, doorY, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHEAST );
		else
			SetTileAt( doorX + 1, doorY, Tile::Tile_WALL_EAST );
	}
	else if ( dir == Dir_SOUTH )
	{
		if ( GetTileAt( doorX, doorY ).z > GetTileAt( doorX, doorY + 1 ).z )
		{
			SetTileAt( doorX, doorY + 1, Tile::Tile_STAIR_BASE_NORTH );
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_TOP_NORTH );
		}
		else if ( GetTileAt( doorX, doorY ).z < GetTileAt( doorX, doorY + 1 ).z )
		{
			SetTileAt( doorX, doorY + 1, Tile::Tile_STAIR_TOP_SOUTH  );
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_BASE_SOUTH );
		}
		else
		{
			// Doors
			if ( GetTileAt( doorX, doorY + 1 ).mRoomTemplate->HasStyleForUsage( Tile::Tile_DOOR ) && RandomPercentCheck( mDoorChance ) )
			{
				SetTileAt( doorX, doorY + 1, Tile::Tile_DOOR_SOUTH );
			}
			else
				SetTileAt( doorX, doorY + 1, Tile::Tile_FLOOR );
		}

		if ( GetTileAt( doorX - 1, doorY ).mType == Tile::Tile_WALL_NORTH )
			SetTileAt( doorX - 1, doorY, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHWEST );
		else
			SetTileAt( doorX - 1, doorY, Tile::Tile_WALL_WEST );

		if ( GetTileAt( doorX + 1, doorY + 1 ).mType == Tile::Tile_WALL_SOUTH )
			SetTileAt( doorX + 1, doorY + 1, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHEAST );
		else
			SetTileAt( doorX + 1, doorY + 1, Tile::Tile_WALL_EAST );

		if ( GetTileAt( doorX - 1, doorY + 1 ).mType == Tile::Tile_WALL_SOUTH )
			SetTileAt( doorX - 1, doorY + 1, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHWEST );
		else
			SetTileAt( doorX - 1, doorY + 1, Tile::Tile_WALL_WEST );

		if ( GetTileAt( doorX + 1, doorY ).mType == Tile::Tile_WALL_NORTH )
			SetTileAt( doorX + 1, doorY, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHEAST );
		else
			SetTileAt( doorX + 1, doorY, Tile::Tile_WALL_EAST );
	}
	else if ( dir == Dir_WEST )
	{
		if ( GetTileAt( doorX, doorY ).z > GetTileAt( doorX + 1, doorY ).z )
		{
			SetTileAt( doorX + 1, doorY, Tile::Tile_STAIR_BASE_EAST );
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_TOP_EAST );
		}
		else if ( GetTileAt( doorX, doorY ).z < GetTileAt( doorX + 1, doorY ).z )
		{
			SetTileAt( doorX + 1, doorY, Tile::Tile_STAIR_TOP_WEST );
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_BASE_WEST );
		}
		else
		{
			// Doors
			if ( GetTileAt( doorX + 1, doorY ).mRoomTemplate->HasStyleForUsage( Tile::Tile_DOOR ) && RandomPercentCheck( mDoorChance ) )
			{
				SetTileAt( doorX + 1, doorY, Tile::Tile_DOOR_WEST );
			}
			else
				SetTileAt( doorX + 1, doorY, Tile::Tile_FLOOR );
		}

		if ( GetTileAt( doorX + 1, doorY - 1 ).mType == Tile::Tile_WALL_WEST )
			SetTileAt( doorX + 1, doorY - 1, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHWEST );
		else
			SetTileAt( doorX + 1, doorY - 1, Tile::Tile_WALL_SOUTH );

		if ( GetTileAt( doorX + 1, doorY + 1 ).mType == Tile::Tile_WALL_WEST )
			SetTileAt( doorX + 1, doorY + 1, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHWEST );
		else
			SetTileAt( doorX + 1, doorY + 1, Tile::Tile_WALL_NORTH );

		if ( GetTileAt( doorX, doorY - 1 ).mType == Tile::Tile_WALL_EAST )
			SetTileAt( doorX, doorY - 1, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHEAST );
		else
			SetTileAt( doorX, doorY - 1, Tile::Tile_WALL_SOUTH );

		if ( GetTileAt( doorX, doorY + 1 ).mType == Tile::Tile_WALL_EAST )
			SetTileAt( doorX, doorY + 1, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHEAST );
		else
			SetTileAt( doorX, doorY + 1, Tile::Tile_WALL_NORTH );
	}
	else if ( dir == Dir_EAST )
	{
		if ( GetTileAt( doorX, doorY ).z < GetTileAt( doorX - 1, doorY ).z )
		{
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_BASE_EAST );
			SetTileAt( doorX - 1, doorY, Tile::Tile_STAIR_TOP_EAST );
		}
		else if ( GetTileAt( doorX, doorY ).z > GetTileAt( doorX - 1, doorY ).z )
		{
			SetTileAt( doorX, doorY, Tile::Tile_STAIR_TOP_WEST );
			SetTileAt( doorX - 1, doorY, Tile::Tile_STAIR_BASE_WEST );
		}
		else
		{
			// Doors
			if ( GetTileAt( doorX - 1, doorY ).mRoomTemplate->HasStyleForUsage( Tile::Tile_DOOR ) && RandomPercentCheck( mDoorChance ) )
			{
				SetTileAt( doorX - 1, doorY, Tile::Tile_DOOR_EAST );
			}
			else
				SetTileAt( doorX - 1, doorY, Tile::Tile_FLOOR );
		}

		if ( GetTileAt( doorX - 1, doorY - 1 ).mType == Tile::Tile_WALL_EAST )
			SetTileAt( doorX - 1, doorY - 1, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHEAST );
		else
			SetTileAt( doorX - 1, doorY - 1, Tile::Tile_WALL_SOUTH );

		if ( GetTileAt( doorX - 1, doorY + 1 ).mType == Tile::Tile_WALL_EAST )
			SetTileAt( doorX - 1, doorY + 1, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHEAST );
		else
			SetTileAt( doorX - 1, doorY + 1, Tile::Tile_WALL_NORTH );

		if ( GetTileAt( doorX, doorY - 1 ).mType == Tile::Tile_WALL_WEST )
			SetTileAt( doorX, doorY - 1, Tile::Tile_WALL_CORNER_OUTSIDE_SOUTHWEST );
		else
			SetTileAt( doorX, doorY - 1, Tile::Tile_WALL_SOUTH );

		if ( GetTileAt( doorX, doorY + 1 ).mType == Tile::Tile_WALL_WEST )
			SetTileAt( doorX, doorY + 1, Tile::Tile_WALL_CORNER_OUTSIDE_NORTHWEST );
		else
			SetTileAt( doorX, doorY + 1, Tile::Tile_WALL_NORTH );
	}
}
//---------------------------------------
void DungeonGenerator::GenerateSpawnData()
{
	for ( auto itr = mRooms.begin(); itr != mRooms.end(); ++itr )
	{
		Room& r = **itr;
		Tile& t0 = GetTileAt( r.x, r.y );
		if ( t0.mRoomTemplate )
			t0.mRoomTemplate->ResetRules();

		std::vector< Tile* > roomTiles;

		for ( int y = r.y; y < r.y + r.GetHeight(); ++y )
		{
			for ( int x = r.x; x < r.x + r.GetWidth(); ++x )
			{
				roomTiles.push_back( &GetTileAt( x, y ) );
			}
		}

		// Shuffle the tiles of the room to remove left-to-right top-to-bottom bias
		std::random_shuffle( roomTiles.begin(), roomTiles.end() );

		// Get world geometry and objects to spawn
		for ( auto tile = roomTiles.begin(); tile != roomTiles.end(); ++tile )
		{
			Tile& t = **tile;
			if ( t.mRoomTemplate )
			{
				t.mStyle = t.mRoomTemplate->GetStyle( &t );
				if ( !t.mBlockObjectSpawn )
					t.mObject = t.mRoomTemplate->GetObject( &t );
			}
		}
	}
}
//---------------------------------------
void DungeonGenerator::LockRandomDoors()
{
	for ( auto itr = mDoors.begin(); itr != mDoors.end(); ++itr )
	{
		Tile& tile = **itr;
		tile.mLocked = RandomPercentCheck( mChanceToLock );
	}
}
//---------------------------------------
void DungeonGenerator::GatherDoors()
{
	for ( auto itr = mTiles.begin(); itr != mTiles.end(); ++itr )
	{
		Tile& t = *itr;
		if ( t.GetUsageId() == Tile::Tile_DOOR_FRAME )
			mDoors.push_back( &t );
	}
}
//---------------------------------------
void DungeonGenerator::FloodFill( int x, int y, int initId, int destId )
{
	Tile& t = GetTileAt( x, y );
	if (
		( ( t.mType > Tile::Tile_NONE && t.mType < Tile::Tile_WALL ) ||
		( t.GetUsageId() == Tile::Tile_DOOR_FRAME && !t.mLocked ) )
		&& t.mSectorId == initId && initId != destId )
	{
		t.mSectorId = destId;
		t.mRoom->mSectorId = destId;

		mSectorsToVisit.insert( destId );
		
		// Debug colors
		mSectorColors[ destId ] = Color( RNG::RandomUnit(), RNG::RandomUnit(), RNG::RandomUnit() );
		if ( t.mType == Tile::Tile_FLOOR )
			mTilesBySector[ destId ].push_back( &t );

		FloodFill( x + 1, y, initId, destId );
		FloodFill( x - 1, y, initId, destId );
		FloodFill( x, y + 1, initId, destId );
		FloodFill( x, y - 1, initId, destId );
	}
}
//---------------------------------------
void DungeonGenerator::CalculateSectors()
{
	// Need to have doors locked before generating connectivity
	LockRandomDoors();

	// Generate connectivity
	int nextId = 1;
	for ( int y = 0; y < mHeight; ++y )
	{
		for ( int x = 0; x < mWidth; ++x )
		{
			Tile& tile = GetTileAt( x, y );
			if ( tile.mType != Tile::Tile_NONE && tile.mSectorId == 0 )
			{
				FloodFill( x, y, 0, nextId++ );
			}
		}
	}
	DebugPrintf( "Sectors Created: %d\n", mSectorColors.size() );

	// Sector id : connected sector ids
	std::map< int, std::vector< int > > sectorMapping;
	std::map< int, Tile* > sectorDoors;
	// Build mapping of sectors to adjacency lists
	for ( auto itr = mDoors.begin(); itr != mDoors.end(); ++itr )
	{
		Tile& tile = **itr;
		
		if ( tile.mLocked )
		{
			int x = tile.x;
			int y = tile.y;
			Tile* startTile = 0;
			Tile* endTile = 0;
			int startId = 0;
			int endId = 0;
			if ( tile.mType == Tile::Tile_DOOR_EAST )
			{
				startTile = &GetTileAt( x + 1, y );
				endTile   = &GetTileAt( x - 1, y );
			}
			else if ( tile.mType == Tile::Tile_DOOR_WEST )
			{
				startTile = &GetTileAt( x - 1, y );
				endTile   = &GetTileAt( x + 1, y );
			}
			else if ( tile.mType == Tile::Tile_DOOR_NORTH )
			{
				startTile = &GetTileAt( x, y + 1 );
				endTile   = &GetTileAt( x, y - 1 );
			}
			else if ( tile.mType == Tile::Tile_DOOR_SOUTH )
			{
				startTile = &GetTileAt( x, y - 1 );
				endTile   = &GetTileAt( x, y + 1 );
			}
			startId = startTile->mSectorId;
			endId = endTile->mSectorId;
			DebugPrintf( "Connection between %d and %d\n", startId, endId );
			sectorMapping[ startId ].push_back( endId );
			sectorMapping[ endId ].push_back( startId );
			sectorDoors[ startId ] = startTile;
			sectorDoors[ endId ] = endTile;
			// Needed so the minimap draws with the correct color
			tile.mSectorId = startId;
		}
	}

	// Debug
	DebugPrintf( "Sector Connections Results\n" );
	for ( auto itr = sectorMapping.begin(); itr != sectorMapping.end(); ++itr )
	{
		DebugPrintf( "[%d] :", itr->first );
		for ( auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr )
		{
			DebugPrintf( " %d", *jtr );
			if ( jtr + 1 != itr->second.end() )
				DebugPrintf( "," );
		}
		DebugPrintf( "\n" );
	}

	std::stack< int > connectionStack;
	mOrderOfVisitation.clear();
	Tile& startTile = GetTileAt( (int) ( mEntranceLocation.x ), (int) ( mEntranceLocation.z ) );
	int startSector = startTile.mSectorId;
	mOrderOfVisitation.push_back( startSector );
	int visitIndex = -1;
	DebugPrintf( "Visit %d\n", startSector );
	while ( !mSectorsToVisit.empty() )
	{
		// Remove sectors we already visited from this sectors adjacency list
		for ( auto itr = mOrderOfVisitation.begin(); itr != mOrderOfVisitation.end(); ++itr )
		{
			std::vector< int >& neighbors = sectorMapping[ startSector ];
			for ( auto jtr = neighbors.begin(); jtr != neighbors.end(); ++jtr )
			{
				if ( *itr == *jtr )
				{
					*jtr = -1;
				}
			}
			neighbors.erase(
			std::remove_if( neighbors.begin(), neighbors.end(), []( int i )
			{
				return i == -1;
			}), neighbors.end() );
		}

		// Still more adjacent sectors to visit
		if ( !sectorMapping[ startSector ].empty() )
		{
			// Get random connection
			int index = RNG::RandomIndex( sectorMapping[ startSector ].size() );
			int randomSectorId = sectorMapping[ startSector ][ index ];

			connectionStack.push( startSector );

			// Place key in startSector for randomSectorId
			int keySpawnSector = mOrderOfVisitation[ visitIndex++ + 1 ];
			Tile* doorTile = sectorDoors[ randomSectorId ];
			std::vector< Tile* > tilesByDistance;
			for ( auto itr = mTilesBySector[ keySpawnSector ].begin(); itr != mTilesBySector[ keySpawnSector ].end(); ++itr )
			{
				tilesByDistance.push_back( *itr );
			}
			// farthest to nearest
			std::sort( tilesByDistance.begin(), tilesByDistance.end(), [ &doorTile ]( Tile* A, Tile* B ) -> bool
			{
				const int d1 = GetManhattanDistance( doorTile->x, doorTile->y, A->x, A->y );
				const int d2 = GetManhattanDistance( doorTile->x, doorTile->y, B->x, B->y );
				return d1 > d2;
			});
			unsigned lowerIndex = (unsigned) ( tilesByDistance.size() * 0.1f );
			Tile* t = tilesByDistance[ RNG::RandomIndex( lowerIndex + 1 ) ];
			t->mBlockObjectSpawn = true;
			int keyId = randomSectorId;
			Key* key = new Key( keyId );
			const Key::KeyStyle& keyStyle = Key::GetRandomKeyStyle();
			key->SetKeyStyle( keyStyle );
			key->SetLocation( glm::vec3( t->x, t->z + 0.5f, t->y ) );
			key->SetColor( mSectorColors[ randomSectorId ] );
			mKeysToSpawn[ randomSectorId ] = key;
			DebugPrintf( "Creating Key to %d in %d\n", randomSectorId, keySpawnSector );

			DebugPrintf( "Visit %d\n", randomSectorId );
			mOrderOfVisitation.push_back( randomSectorId );
			
			// Remove sector from list of sectors to visit
			auto itr = std::find( mSectorsToVisit.begin(), mSectorsToVisit.end(), startSector );
			if ( itr != mSectorsToVisit.end() )
				mSectorsToVisit.erase( itr );
			sectorMapping[ startSector ].erase( sectorMapping[ startSector ].begin() + index );

			// Same thing starting in the sector we just picked
			startSector = randomSectorId;
		}
		// Backtrack
		else if ( !connectionStack.empty() )
		{
			startSector = connectionStack.top();
			connectionStack.pop();
		}
		// Dead end, try to start from a random location
		else
		{
			int index = RNG::RandomIndex( mSectorsToVisit.size() );
			auto itr = mSectorsToVisit.begin();
			std::advance( itr, index );
			startSector = *itr;

			mSectorsToVisit.erase( itr );
			mOrderOfVisitation.push_back( startSector );
		}
	}
}
//---------------------------------------
void DungeonGenerator::Draw( Window* window )
{
	// Debug
	if ( DebugSectors )
	{
		for ( auto itr = mTilesBySector.begin(); itr != mTilesBySector.end(); ++itr )
		{
			Texture2D::Unbind();
			window->SetDrawColor( mSectorColors[ itr->first ] );
			for ( auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr )
			{
				Tile* t = *jtr;
				window->DrawQuad( (float) t->x, (float) t->y, t->z + 0.005f, 1.0f, 1.0f );
			}
		}

		// Critical path debug
		float x = 0.0f, y = 220.0f;
		for ( auto itr = mSectorColors.begin(); itr != mSectorColors.end(); ++itr )
		{
			window->DrawDebugTextFmt( x, y, itr->second, "%d", itr->first );
			y += 24.0f;
		}
		for ( auto itr = mOrderOfVisitation.begin(); itr != mOrderOfVisitation.end(); ++itr )
		{
			window->DrawDebugTextFmt( x, y, mSectorColors[ *itr ], "%d ", *itr );
			x += 48.0f;
		}
	}
}
//---------------------------------------
void DungeonGenerator::SpawnEntities( Game* world )
{
	int startSector;
	int endSector;

	startSector = GetTileAt( (int)( mEntranceLocation.x + 1 ), (int) ( mEntranceLocation.z ) ).mSectorId;
	endSector = GetTileAt( (int) ( mExitLocation.x + 1 ), (int) ( mExitLocation.z ) ).mSectorId;

	DebugPrintf( "Spawn: Starting in %d\n", startSector );
	DebugPrintf( "Spawn: Ending in %d\n", endSector );

	for ( auto itr = mKeysToSpawn.begin(); itr != mKeysToSpawn.end(); ++itr )
	{
		world->AddEntity( itr->second );
	}

	for ( int y = 0; y < mHeight; ++y )
	{
		for ( int x = 0; x < mWidth; ++x )	
		{
			Tile& tile = GetTileAt( x, y );
			const int c = tile.mType;
			const float z = tile.z;
			const float cellW = 1.0f;
			const float cellH = 1.0f;

			if ( tile.mStyle )
			{
				MapTile* e = new MapTile( &tile );
				tile.mStyle->SetupEvents( e );
				world->AddEntity( e );
			}

			if ( tile.mObject )
			{
				SpawnTileObject( world, tile, tile.mObject, 0 );
			}

			if ( tile.GetUsageId() == Tile::Tile_DOOR_FRAME )
			{
				Tile doorTile( tile );
				doorTile.mType = Tile::Tile_DOOR;
				TileStyle* doorStyle = tile.mRoomTemplate->GetStyle( &doorTile );

				// This will happen if a room failed to spawn and there was no other option
				if ( !doorStyle )
					continue;

				Mesh* doorMesh = doorStyle->mMesh;
				Door* d = new Door( &tile, doorMesh );
				d->SetLocation( glm::vec3( x, z, y ) );
				d->SetRotation( glm::vec3( 0, 1.0f, 0 ), glm::radians( tile.GetOrientation() ) );
				doorStyle->SetupEvents( d );
	
				if ( tile.mLocked )
				{
					int startId = 0;
					int endId = 0;
					if ( tile.mType == Tile::Tile_DOOR_EAST )
					{
						startId = GetTileAt( x + 1, y ).mSectorId;
						endId = GetTileAt( x - 1, y ).mSectorId;
					}
					else if ( tile.mType == Tile::Tile_DOOR_WEST )
					{
						startId = GetTileAt( x - 1, y ).mSectorId;
						endId = GetTileAt( x + 1, y ).mSectorId;
					}
					else if ( tile.mType == Tile::Tile_DOOR_NORTH )
					{
						startId = GetTileAt( x, y + 1 ).mSectorId;
						endId = GetTileAt( x, y - 1 ).mSectorId;
					}
					else if ( tile.mType == Tile::Tile_DOOR_SOUTH )
					{
						startId = GetTileAt( x, y - 1 ).mSectorId;
						endId = GetTileAt( x, y + 1 ).mSectorId;
					}

					int id = startId;
					auto itr = mKeysToSpawn.find( endId );
					if ( itr != mKeysToSpawn.end() )
						id = endId;

					if ( !doorStyle->mCanBeLocked )
					{
						if ( doorStyle->mForceLocked )
							d->Lock( 0 );
						mKeysToSpawn[ id ]->Destroy();
					}
					else
					{
						d->Lock( id );
						d->SetColor( mSectorColors[ id ] );
						d->SetKeyName( mKeysToSpawn[ id ]->GetKeyName() );
					}
					tile.mSectorId = id;
				}
				world->AddEntity( d );
			}
		}
	}

	mKeysToSpawn.clear();
}
//---------------------------------------
bool DungeonGenerator::RandomPercentCheck( float percentToBeTrue ) const
{
	const float r = RNG::RandomUnit();
	return percentToBeTrue > 0 && r <= percentToBeTrue ? true : false;
}
//---------------------------------------
void DungeonGenerator::SpawnTileObject( Game* world, Tile& tile, TileObject* obj, Entity* parent )
{
	Entity* e = 0;
	if ( obj->mUsageId == TileObject::Usage_STATIC )
		e = new MapObject( &tile, obj->mMesh );
	else if ( obj->mUsageId == TileObject::Usage_PICKUP )
	{
		TileObject_Pickup* pickup = (TileObject_Pickup*) obj;
		e = Pickup::CreatePickup( pickup->mPickupName );
		
		if ( e )
		{
			//if ( obj->mMesh )
			//	((Pickup*)e)->SetMesh( obj->mMesh );
			((Pickup*)e)->SetLocation( glm::vec3( tile.x, tile.z, tile.y ) + obj->mLocalSpawnOffset );
		}
	}
	else if ( obj->mUsageId == TileObject::Usage_LIGHT )
	{
		TileObject_Light* lightObject = (TileObject_Light*) obj;

		PointLight* light = world->CreateLight( lightObject->mLightColor, lightObject->mIntensity, lightObject->mFalloff, lightObject->mRadius );
		if ( light )
			light->mPosition = glm::vec3( tile.x, tile.z, tile.y ) + obj->mLocalSpawnOffset;

		e = light;
	}
	else if ( obj->mUsageId == TileObject::Usage_ENEMY )
	{
		TileObject_Enemy* enemyObject = (TileObject_Enemy*) obj;
		e = Enemy::CreateEnemy( enemyObject->mEnemyClass );
		if ( e )
			((Actor*)e)->SetSpawnLocation( glm::vec3( tile.x, tile.z, tile.y ) + obj->mLocalSpawnOffset );
	}
	else if ( obj->mUsageId == TileObject::Usage_SPAWNER )
	{
		TileObject_Spawner* spawner = (TileObject_Spawner*) obj;
		e = Enemy::CreateEnemy( spawner->GetObjectToSpawn() );
		if ( e )
			((Actor*)e)->SetSpawnLocation( glm::vec3( tile.x, tile.z, tile.y ) + obj->mLocalSpawnOffset );
	}

	if ( e )
	{
		e->SetParent( parent );
		if ( parent )
			parent->AddAttachment( e );
		obj->SetupEvents( e );
		world->AddEntity( e );
	}

	if ( obj->mAttachment )
	{
		SpawnTileObject( world, tile, obj->mAttachment, e );
	}
}
//---------------------------------------
const Room* DungeonGenerator::GetRoomBySectorId( int sectorId ) const
{
	const Room* room = 0;

	for ( auto itr = mRooms.begin(); itr != mRooms.end(); ++itr )
	{
		if ( (*itr)->mSectorId == sectorId )
		{
			room = *itr;
			break;
		}
	}

	return room;
}
//---------------------------------------