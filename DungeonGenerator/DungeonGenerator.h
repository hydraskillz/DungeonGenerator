/*
 * Author      : Matthew Johnson
 * Date        : 21/Jan/2014
 * Description :
 *   Generates a procedural dungeon.
 *   The generation process involves feminist objects.
 *   These objects when asked to spawn on the map have to check with
 *   their Rules and Values to decide how they Feel. After determining
 *   how they Feel the object may Choose weather or not it wishes to be spawned.
 *   The generator must respect and follow the objects choice.
 */
 
#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <map>
#include <set>
#include "Color.h"
#include "XmlReader.h"
#include "Logger.h"

//---------------------------------------
// Forwards
class Window;
class Texture2D;
class Mesh;
class Entity;
class Game;
class Key;
struct Tile;
class TileGrid;
class Room;
struct DepthValue;

//---------------------------------------
// Rule used for placement of objects in the world
struct Rule
{
	static Rule* CreateRule( const XmlReader::XmlReaderIterator& xmlItr );

	virtual ~Rule() {}
	virtual bool IsValid( Tile* tile ) = 0;
	virtual void Reset() {}
	virtual void NotifySuccess() {}
	virtual Rule* Copy() const = 0;

	TileGrid* mGrid;
};
//---------------------------------------
struct Rule_Random
	: public Rule
{
	Rule_Random( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_Random( *this ); }

protected:
	float mPercentToBeTrue;
};
//---------------------------------------
struct Rule_UsageRule
{
	Rule_UsageRule( const XmlReader::XmlReaderIterator& xmlItr );

protected:
	std::vector< int > mUsages;
};
//---------------------------------------
struct Rule_ObjectRule
{
	Rule_ObjectRule( const XmlReader::XmlReaderIterator& xmlItr );

protected:
	std::vector< std::string > mObjectNames;
};
//---------------------------------------
struct Rule_StyleRule
{
	Rule_StyleRule( const XmlReader::XmlReaderIterator& xmlItr );

protected:
	std::vector< std::string > mStyleNames;
};
//---------------------------------------
struct Rule_SpawnOn
	: public Rule
	, public Rule_UsageRule
{
	Rule_SpawnOn( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile ) = 0;
};
//---------------------------------------
struct Rule_CanSpawnOn
	: public Rule_SpawnOn
{
	Rule_CanSpawnOn( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_CanSpawnOn( *this ); }
};
//---------------------------------------
struct Rule_CanNotSpawnOn
	: public Rule_SpawnOn
{
	Rule_CanNotSpawnOn( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_CanNotSpawnOn( *this ); }
};
//---------------------------------------
struct Rule_MaxCount
	: public Rule
{
	Rule_MaxCount( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_MaxCount( *this ); }
	void Reset();
	void NotifySuccess() { --mCurrentCount; }
	
protected:
	int mMaxCount;
	int mCurrentCount;
};
//---------------------------------------
struct Rule_AdjacentTo
	: public Rule
{
	Rule_AdjacentTo( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile ) = 0;
	void GetTilesToCheck( Tile* tile, std::vector< Tile* >& tilesToCheck );

	enum AdjacentDirection
	{
		AD_N,
		AD_S,
		AD_E,
		AD_W,
		AD_NE,
		AD_NW,
		AD_SE,
		AD_SW,
		AD_COUNT
	};
protected:
	int mMargin;
	bool mDirectionsToCheck[ AD_COUNT ];
	bool mPassValue;
};
//---------------------------------------
struct Rule_NotAdjacentToUsage
	: public Rule_AdjacentTo
	, Rule_UsageRule
{
	Rule_NotAdjacentToUsage( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_NotAdjacentToUsage( *this ); }
	virtual bool CheckTile( Tile* tile, int usage );
};
//---------------------------------------
struct Rule_NotAdjacentToObject
	: public Rule_AdjacentTo
	, public Rule_ObjectRule
{
	Rule_NotAdjacentToObject( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_NotAdjacentToObject( *this ); }
	bool CheckTile( Tile* tile, const std::string& objectName );
};
//---------------------------------------
struct Rule_NotAdjacentToStyle
	: public Rule_AdjacentTo
	, public Rule_StyleRule
{
	Rule_NotAdjacentToStyle( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool IsValid( Tile* tile );
	virtual Rule* Copy() const { return new Rule_NotAdjacentToStyle( *this ); }
	bool CheckTile( Tile* tile, const std::string& styleName );
};
//---------------------------------------
struct Rule_AdjacentToUsage
	: public Rule_NotAdjacentToUsage
{
	Rule_AdjacentToUsage( const XmlReader::XmlReaderIterator& xmlItr );
	virtual Rule* Copy() const { return new Rule_AdjacentToUsage( *this ); }
};
//---------------------------------------
struct Rule_AdjacentToObject
	: public Rule_NotAdjacentToObject
{
	Rule_AdjacentToObject( const XmlReader::XmlReaderIterator& xmlItr );
	virtual Rule* Copy() const { return new Rule_AdjacentToObject( *this ); }
};
//---------------------------------------
struct Rule_AdjacentToStyle
	: public Rule_NotAdjacentToStyle
{
	Rule_AdjacentToStyle( const XmlReader::XmlReaderIterator& xmlItr );
	virtual Rule* Copy() const { return new Rule_AdjacentToStyle( *this ); }
};
//---------------------------------------
struct Rule_DistanceTo
	: public Rule
{
	Rule_DistanceTo( const XmlReader::XmlReaderIterator& xmlItr );
	bool IsValid( Tile* tile );
	virtual bool ShouldCheck( const Tile& tile ) const = 0;
	virtual float GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const = 0;

private:
	float mMinDist;
	float mMaxDist;
};
//---------------------------------------
struct Rule_DistanceToUsage
	: public Rule_DistanceTo
	, public Rule_UsageRule
{
	Rule_DistanceToUsage( const XmlReader::XmlReaderIterator& xmlItr );
	Rule* Copy() const { return new Rule_DistanceToUsage( *this ); }
	bool ShouldCheck( const Tile& tile ) const;
	float GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const;
};
//---------------------------------------
struct Rule_DistanceToObject
	: public Rule_DistanceTo
	, public Rule_ObjectRule
{
	Rule_DistanceToObject( const XmlReader::XmlReaderIterator& xmlItr );
	Rule* Copy() const { return new Rule_DistanceToObject( *this ); }
	bool ShouldCheck( const Tile& tile ) const;
	float GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const;
};
//---------------------------------------
struct Rule_DistanceToStyle
	: public Rule_DistanceTo
	, public Rule_StyleRule
{
	Rule_DistanceToStyle( const XmlReader::XmlReaderIterator& xmlItr );
	Rule* Copy() const { return new Rule_DistanceToStyle( *this ); }
	bool ShouldCheck( const Tile& tile ) const;
	float GetDistanceBetween( const Tile& startTile, const Tile& endTile ) const;
};
//---------------------------------------
struct Rule_ValidDepths
	: public Rule
{
	Rule_ValidDepths( const XmlReader::XmlReaderIterator& xmlItr );
	Rule_ValidDepths( const Rule_ValidDepths& other );
	virtual ~Rule_ValidDepths();
	bool IsValid( Tile* tile );
	Rule* Copy() const { return new Rule_ValidDepths( *this ); }
	void AddValidDepth( DepthValue* depth );

private:
	bool ValidateDepth( int currentDepth ) const;

	std::vector< DepthValue* > mValidDepths; // Depths at which this rule is valid
};
//---------------------------------------
struct Rule_RoomContains
	: public Rule
{
	Rule_RoomContains();
	bool IsValid( Tile* tile );
	virtual bool CheckTile( const Tile& tile ) = 0;
protected:
	bool mPassValue;
};
//---------------------------------------
struct Rule_RoomDoesNotHaveUsage
	: public Rule_RoomContains
	, public Rule_UsageRule
{
	Rule_RoomDoesNotHaveUsage( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool CheckTile( const Tile& tile );
	virtual Rule* Copy() const { return new Rule_RoomDoesNotHaveUsage( *this ); }
};
//---------------------------------------
struct Rule_RoomDoesHaveUsage
	: public Rule_RoomDoesNotHaveUsage
{
	Rule_RoomDoesHaveUsage( const XmlReader::XmlReaderIterator& xmlItr );
	Rule* Copy() const { return new Rule_RoomDoesHaveUsage( *this ); }
};
//---------------------------------------
struct Rule_RoomDoesNotHaveStyle
	: public Rule_RoomContains
	, public Rule_StyleRule
{
	Rule_RoomDoesNotHaveStyle( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool CheckTile( const Tile& tile );
	virtual Rule* Copy() const { return new Rule_RoomDoesNotHaveStyle( *this ); }
};
//---------------------------------------
struct Rule_RoomDoesHaveStyle
	: public Rule_RoomDoesNotHaveStyle
{
	Rule_RoomDoesHaveStyle( const XmlReader::XmlReaderIterator& xmlItr );
	Rule* Copy() const { return new Rule_RoomDoesHaveStyle( *this ); }
};
//---------------------------------------
struct Rule_RoomDoesNotHaveObject
	: public Rule_RoomContains
	, public Rule_ObjectRule
{
	Rule_RoomDoesNotHaveObject( const XmlReader::XmlReaderIterator& xmlItr );
	virtual bool CheckTile( const Tile& tile );
	virtual Rule* Copy() const { return new Rule_RoomDoesNotHaveObject( *this ); }
};
//---------------------------------------
struct Rule_RoomDoesHaveObject
	: public Rule_RoomDoesNotHaveObject
{
	Rule_RoomDoesHaveObject( const XmlReader::XmlReaderIterator& xmlItr );
	Rule* Copy() const { return new Rule_RoomDoesHaveObject( *this ); }
};
//---------------------------------------


//---------------------------------------
// An object that has Rules
struct RuledObject
{
	virtual ~RuledObject();

	// Cases Reset() to be called on all Rules
	// This will reset any cached or tracking data in the Rules
	// to be set to their initial values
	virtual void ResetRules();

	// Add a new Rule to this object
	// This object will take ownership of the pointer
	virtual void AddRule( Rule* rule );
	virtual void AddRules( const std::vector< Rule* >& rules );

	// Check the given Tile against this objects Rules
	virtual bool ValidateRules( Tile* tile );

	// Lets the Rules know that all checks passed and they should
	// do an internal update of their tracking data
	void NotifySuccess();

	// Load Rule tags from someTag
	// Structure is as so:
	// <someTag>
	//   <Rule />
	// </someTag>
	template< typename TRuledObject >
	void LoadRulesFromXML( const XmlReader::XmlReaderIterator& xmlItr, TileGrid* ownerGrid, const std::map< std::string, TRuledObject* >& objectMap )
	{
		for ( XmlReader::XmlReaderIterator ruleItr = xmlItr.NextChild( "Rule" );
			ruleItr.IsValid(); ruleItr = ruleItr.NextSibling( "Rule" ) )
		{
			Rule* rule = Rule::CreateRule( ruleItr );
			rule->mGrid = ownerGrid;
			mRules.push_back( rule );
		}

		// Copy rules
		std::vector< std::string > objectsToCopyFrom;
		xmlItr.GetAttributeAsCSV( "extendsRules", objectsToCopyFrom );
		for ( auto objectCopyItr = objectsToCopyFrom.begin(); objectCopyItr != objectsToCopyFrom.end(); ++objectCopyItr )
		{
			TRuledObject* objectToCopy = objectMap.at( *objectCopyItr );
			if ( objectToCopy )
			{
				CopyRulesFrom( objectToCopy );
			}
			else
			{
				WarnFail( "Could not find Object of name '%s'. Did you define it after this Object?\n", objectCopyItr->c_str() );
			}
		}
	}

	// Append this objects rules with copies of obj rules
	void CopyRulesFrom( RuledObject* obj );
protected:
	std::vector< Rule* > mRules;
};

//---------------------------------------
// Holds data about events
struct EventObject
{
	void LoadEventsFromXML( const XmlReader::XmlReaderIterator& xmlItr );
	void SetupEvents( Entity* entity ) const;

	// <type, <tags>>
	std::map< int, std::vector< unsigned int > > mSignals;
	// <tag, <commands>>
	std::map< unsigned int, std::vector< std::string > > mSlots;
};


//---------------------------------------
// User defined object that can spawn in the map
struct TileObject
	: public RuledObject
	, public EventObject
{
	enum Usage
	{
		Usage_STATIC,
		Usage_DYNAMIC,
		Usage_PICKUP,
		Usage_LIGHT,
		Usage_ENEMY,
		Usage_SPAWNER,
	};

	static int GetUsageIdFromString( const std::string& str );
	
	int mUsageId;		// How this object is used
	Mesh* mMesh;
	std::string mName;
	glm::vec3 mLocalSpawnOffset;
	TileObject* mAttachment;
};

struct TileObject_Pickup
	: public TileObject
{
	std::string mPickupName;
};

struct TileObject_Light
	: public TileObject
{
	Color mLightColor;
	float mIntensity;
	float mRadius;
	float mFalloff;
};

struct TileObject_Enemy
	: public TileObject
{
	std::string mEnemyClass;
};

struct SpawnList
{
	const std::string& GetRandomObject() const;

	std::vector< std::string > mList;
};

// Just spawns enemies right now
struct TileObject_Spawner
	: public TileObject
{
	const std::string& GetObjectToSpawn() const;

	SpawnList* mList;
};



//---------------------------------------
// User defined style used to render the tile
struct TileStyle
	: public RuledObject
	, public EventObject
{
	static int GetUsageIdFromString( const std::string& str );

	int mUsageId;		// What tile type this style can be applied to
	Mesh* mMesh;
	std::string mName;
	bool mCanBeLocked;	// Used by doors
	bool mForceLocked;
};


//---------------------------------------
// Holds data from a <UseXXX>
struct Useable
	: public RuledObject
{
	void ResetRules();

	RuledObject* mObject;
};


//---------------------------------------
// Depth value used to determine if a room
// can be created at the current depth
struct DepthValue
{
	virtual bool IsValidAtDepth( int depth ) const = 0;
	virtual DepthValue* Copy() const = 0;
};
//---------------------------------------
struct DepthValue_All
	: public DepthValue
{
	bool IsValidAtDepth( int depth ) const { return true; }
	DepthValue* Copy() const { return new DepthValue_All( *this ); }
};
//---------------------------------------
struct DepthValue_Single
	: public DepthValue
{
	DepthValue_Single( int depth ) : mDepth( depth ) {}
	virtual bool IsValidAtDepth( int depth ) const { return depth == mDepth; }
	virtual DepthValue* Copy() const { return new DepthValue_Single( *this ); }
	void SetValidDepth( int depth ) { mDepth = depth; }

protected:
	int mDepth;
};
//---------------------------------------
struct DepthValue_Range
	: public DepthValue
{
	DepthValue_Range( int min, int max ) : mMinDepth( min ), mMaxDepth( max ) {}
	bool IsValidAtDepth( int depth ) const { return depth >= mMinDepth && depth <= mMaxDepth; }
	DepthValue* Copy() const { return new DepthValue_Range( *this ); }
	void SetDepthRange( int min, int max ) { mMinDepth = min; mMaxDepth = max; }

private:
	int mMinDepth, mMaxDepth;
};
//---------------------------------------
struct DepthValue_LessThan
	: public DepthValue_Single
{
	DepthValue_LessThan( int depth ) : DepthValue_Single( depth ) {}
	bool IsValidAtDepth( int depth ) const { return depth <= mDepth; }
	DepthValue* Copy() const { return new DepthValue_LessThan( *this ); }
};
//---------------------------------------
struct DepthValue_GreaterThan
	: public DepthValue_Single
{
	DepthValue_GreaterThan( int depth ) : DepthValue_Single( depth ) {}
	bool IsValidAtDepth( int depth ) const { return depth >= mDepth; }
	DepthValue* Copy() const { return new DepthValue_GreaterThan( *this ); }
};


//---------------------------------------
// Template for room creation
// Holds data for generating a room
class RoomTemplate
	: public RuledObject
{
	friend class DungeonGenerator;
public:
	RoomTemplate();

	void CopyDataFrom( RoomTemplate* room );

	TileStyle* GetStyle( Tile* tile );
	void AddStyle( Useable* style );
	bool HasStyleForUsage( int usage ) const;
	TileObject* GetObject( Tile* tile );
	void AddObject( Useable* object );
	void ResetRules();
	void ResetObjects();
	void SetMinSize( int x, int y );
	void SetMaxSize( int x, int y );

private:
	std::map< int, std::vector< Useable* > > mStyles;		// Styles that can be applied to Tiles in this Room
	std::vector< Useable* > mObjects;						// Objects that can spawn in this Room
	int mMinRoomSizeX, mMinRoomSizeY;						// If > 0 overrides area room sizes
	int mMaxRoomSizeX, mMaxRoomSizeY;
};

//---------------------------------------
// A tile is a single cell in the map grid
// It contains the type of the cell (see TileType) and the vertical location of the tile
// Also contains detailing information for rendering
struct Tile
{
	enum TileType
	{
		Tile_NONE,			// Empty cell
		Tile_FLOOR,
		Tile_STAIR_BASE,
		Tile_STAIR_TOP,
		Tile_STAIR_BASE_NORTH,	// Stairs
		Tile_STAIR_BASE_SOUTH,
		Tile_STAIR_BASE_EAST,
		Tile_STAIR_BASE_WEST,
		Tile_STAIR_TOP_NORTH,
		Tile_STAIR_TOP_SOUTH,
		Tile_STAIR_TOP_EAST,
		Tile_STAIR_TOP_WEST,
		Tile_EXIT,			// Exit
		Tile_ENTRANCE,		// Entrance
		Tile_WALL,			// Beyond this value are walls, before are floors
		Tile_WALL_CORNER,
		Tile_WALL_CORNER_NORTHWEST,
		Tile_WALL_CORNER_NORTHEAST,
		Tile_WALL_CORNER_SOUTHWEST,
		Tile_WALL_CORNER_SOUTHEAST,
		Tile_WALL_CORNER_OUTSIDE,
		Tile_WALL_CORNER_OUTSIDE_NORTHWEST,
		Tile_WALL_CORNER_OUTSIDE_NORTHEAST,
		Tile_WALL_CORNER_OUTSIDE_SOUTHWEST,
		Tile_WALL_CORNER_OUTSIDE_SOUTHEAST,
		Tile_WALL_NORTH,
		Tile_WALL_SOUTH,
		Tile_WALL_EAST,
		Tile_WALL_WEST,
		Tile_DOOR,			// Doors
		Tile_DOOR_FRAME,
		Tile_DOOR_NORTH,
		Tile_DOOR_SOUTH,
		Tile_DOOR_EAST,
		Tile_DOOR_WEST,
	};

	// Returned from GetTileAt() as invalid bounds flag
	static Tile NULL_TILE;

	Tile()
		: mType( Tile_NONE )
		, mSectorId( 0 )
		, z( 0 )
		, mRoomTemplate( 0 )
		, mStyle( 0 )
		, mObject( 0 )
		, mRoom( 0 )
		, mRevealed( false )
		, mLocked( false )
		, mBlockObjectSpawn( false )
	{}

	void SetLocation( int _x, int _y ) { x = _x; y = _y; }
	int GetUsageId() const;
	float GetOrientation() const;
	bool HasObjectOfName( const std::string& name ) const;
	bool HasStyleOfName( const std::string& name ) const;
	bool CanBeLocked() const;

	int x, y;				// Array Location
	float z;				// Height
	int mType;				// TileType
	int mSectorId;			// Connectivity Info
	bool mRevealed;			// Used by Minimap
	bool mLocked;			// Used by Doors
	bool mBlockObjectSpawn;	// Set internally to prevent objects from spawning on this tile

	RoomTemplate* mRoomTemplate;
	Room* mRoom;			// This is only valid during generation
	TileStyle* mStyle;
	TileObject* mObject;
};

//---------------------------------------
// A 2D grid of Tiles
// Used as a base for classes that need a 2D tile grid
class TileGrid
{
public:
	TileGrid()
		: mWidth( 0 )
		, mHeight( 0 )
	{}
	TileGrid( int w, int h )
	{ Resize( w, h ); }

	void Resize( int w, int h )
	{
		mWidth = w; mHeight = h;
		mTiles.resize( w * h );
		for ( int y = 0; y < mHeight; ++y )
			for ( int x = 0; x < mWidth; ++x )
				GetTileAt( x, y ).SetLocation( x, y );
	}

	void Clear()
	{ mTiles.clear(); }

	// Tile helpers
	Tile& GetTileAt( int x, int y )
	{
		const int i = x * mHeight + y;
		if ( i < 0 || i >= (int) mTiles.size() )
			return Tile::NULL_TILE;
		return mTiles[ x * mHeight + y ];
	}
	void SetTileAt( int x, int y, int value )
	{
		mTiles[ x * mHeight + y ].mType = value;
		mTiles[ x * mHeight + y ].SetLocation( x, y );
	}
	void SetTileAt( int x, int y, const Tile& value )
	{
		mTiles[ x * mHeight + y ] = value;
		mTiles[ x * mHeight + y ].SetLocation( x, y );
	}

	inline int GetWidth() const { return mWidth; }
	inline int GetHeight() const { return mHeight; }

protected:
	int mWidth, mHeight;
	std::vector< Tile > mTiles;
};

//---------------------------------------
// Rooms are generated then placed onto the grid
// If a room does not fit it is discarded
// The data in the room class is used only during generation
class Room
	: public TileGrid
{
public:
	Room() {}
	Room( int w, int h, std::vector< Tile >& tiles )
		: TileGrid( w, h )
	{
		mTiles = tiles;
	}

	int x, y;					// Location of top left of this grid in the parent grid
	int mSectorId;				// The sector id for the tiles in this room
	RoomTemplate* mTemplate;	// Template used to create this room
};

//---------------------------------------
// The Dungeon Generator
// Use Generate() to create the dungeon!
class DungeonGenerator
	: public TileGrid
{
public:
	// Initialized to default values
	DungeonGenerator();
	DungeonGenerator( int width, int height, int maxRoomCount, int minRoomWidth, int maxRoomWidth, int minRoomHeight, int maxRoomHeight );
	~DungeonGenerator();

	// Use to set values before generation
	void Init( int width, int height, int maxRoomCount, int minRoomWidth, int maxRoomWidth, int minRoomHeight, int maxRoomHeight );

	// The chance a room will change in height
	// v = [0,1]
	void SetVerticalness( float v ) { mVerticalChance = v; }
	// Determines which direction the room spawns in the vertical axis
	// u = [0,1] d = [0,1]
	void SetVerticalBias( float u, float d ) { mVerticalBiasUp = u; mVerticalBiasDown = d; }
	// The chance a door will spawn between rooms
	// c = [0,1]
	void SetDoorChance( float c ) { mDoorChance = c; }
	// The chance a door will be locked
	// c = [0,1]
	void SetDoorLockChance( float c ) { mChanceToLock = c; }
	// Name of the dungeon
	void SetName( const char* name ) { mName = name; }
	const char* GetName() const { return mName.c_str(); }
	const char* GetFloorName() const { return mFloorName.c_str(); }
	int GetCurrentDepth() const { return mCurrentDepth; }

	// Generate a random dungeon
	void Generate();

	// An ASCII string of the grid
	std::string ToText();

	// Debug info drawing
	void Draw( Window* window );

	// Add an empty room template
	RoomTemplate& CreateRoomTemplate()
	{
		mRoomTemplates.push_back( new RoomTemplate() );
		return *mRoomTemplates.back();
	}

	// Useful locations
	glm::vec3 GetStartLocation() const { return mEntranceLocation; }
	glm::vec3 GetExitLocation() const { return mExitLocation; }
	void GetDoorLocations( std::vector< glm::vec3 >& doorLocations ) const;

	// Spawn entities in the world
	void SpawnEntities( Game* world );

	const Color& GetColorForSectorId( int sectorId ) const
	{
		auto itr = mSectorColors.find( sectorId );
		if ( itr != mSectorColors.end() )
			return itr->second;
		return Color::GREY;
	}

	bool DebugSectors;
private:
	// Used for creating connections between rooms
	enum TileDir
	{
		Dir_INVALID,
		Dir_NORTH,
		Dir_SOUTH,
		Dir_EAST,
		Dir_WEST,
	};

	

	// Generate a random room
	void GenerateRoom( Room& room );
	// Add a room to the map
	void AddRoom( Room& room, int x, int y, float z );
	// Create an empty dungeon
	void Clear();
	// Place entrance - must be called before CalculateSectors()
	void PlaceEntrance();
	// Place exit - must be called after CalculateSectors()
	void PlaceExit();
	// Get a location where a door could be placed
	void GetDoorLocation( int& x, int& y );
	// Get the direction the door should open
	int GetDoorDirection( int x, int y );
	// Try to place a room
	bool CanRoomFitHere( int x, int y, int w, int h, float z );
	// Create a connection between two rooms
	// This function will mutate tiles around the connection so that they look correct
	void ConnectRooms( int doorX, int doorY, int dir );
	// Generate what objects and world geometry should spawn on each tile
	void GenerateSpawnData();
	// Create a critical path through the level
	void CalculateSectors();
	// Used by CalculateSectors() to determine connectivity
	void FloodFill( int x, int y, int initId, int destId );
	// Locks random doors in the map
	void LockRandomDoors();
	// Cache all the door tiles
	void GatherDoors();
	// Utility for random checks
	bool RandomPercentCheck( float percentToBeTrue ) const;
	// Spawns an object and all of its attachments
	void SpawnTileObject( Game* world, Tile& tile, TileObject* obj, Entity* parent=0 );
	// Get a room by its id. Returns null if no rooms in the sector
	const Room* GetRoomBySectorId( int sectorId ) const;

	// Dimensions
	int mMaxRoomCount;
	int mMinRoomSizeX, mMinRoomSizeY;
	int mMaxRoomSizeX, mMaxRoomSizeY;

	// Variance
	float mVerticalChance;
	float mVerticalBiasUp;
	float mVerticalBiasDown;
	float mDirectionBias[2];
	float mDoorChance;
	float mChanceToLock;

	// Important cached locations
	glm::vec3 mEntranceLocation;
	glm::vec3 mExitLocation;

	// Generation
	std::vector< RoomTemplate* > mRoomTemplates;
	RoomTemplate mDummyRoomTmpl;
	std::vector< Room* > mRooms;
	std::vector< Tile* > mDoors;
	std::map< int, std::vector< Tile* > > mTilesBySector;
	std::map< int, Key* > mKeysToSpawn;
	std::set< int > mSectorsToVisit;
	std::vector< int > mOrderOfVisitation;

	// Debug
	std::map< int, Color > mSectorColors;

	// Naming
	std::string mName;
	std::string mFloorName;
	int mCurrentDepth;
};