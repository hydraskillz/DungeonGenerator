#include "Game.h"

#include "XmlReader.h"

#include <SDL.h>
#include <iostream>
#include <fstream>
#include <string>

#include "Texture.h"
#include "Object.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Mesh.h"
#include "BitmapFont.h"
#include "Util.h"
#include "GameLog.h"
#include "Shader.h"
#include "Effect.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Key.h"
#include "Enemy.h"
#include "Weapon.h"

//---------------------------------------
// Static
Game* Game::mInstance;
const float Game::MAX_RELEVANT_DISTANCE = 35.0f;
//---------------------------------------
Game* Game::Get()
{
	return mInstance;
}
//---------------------------------------


//---------------------------------------
Game::Game()
{
	Game::mInstance = this;

	mWindow.Init();
	mRunning = false;
	mMinimapIsFullscreen = false;
	mShowHelp = false;
	mHideHUD = false;
	mHasFocus = false;

	// Camera
	mCamera.width = 1280;
	mCamera.height = 720;
	mCamera.aspectRatio = 1280.0f / 720.0f;

	mCamera.position.x = 0;
	mCamera.position.y = 0;
	mCamera.position.z = 0;

	mCamera.map = &mGenerator;

	mWindow.SetCamera( &mCamera );

	LoadAssets();
	
	//Weapon* shotty = Weapon::CreateWeapon( "shotgun" );
	//mPlayer.GiveWeapon( shotty );
	
	mMinimap.SetMap( &mGenerator );

	// Lighting shaders
	char* basicVertTxt;
	char* basicFragTxt;
	unsigned len;
	OpenDataFile( "shaders/basic.vert.glsl", basicVertTxt, len );
	OpenDataFile( "shaders/basic.frag.glsl", basicFragTxt, len );
	Shader basicVert( Shader::ST_VERTEX, basicVertTxt );
	Shader basicFrag( Shader::ST_FRAGMENT, basicFragTxt );

	mBasicLightingEffect.SetShaders( basicVert, basicFrag );

	mGlobalLight.Color.SetLocation( mBasicLightingEffect.GetUniformLocation( "uGlobalAmbientLight.Color" ) );
	mGlobalLight.Intensity.SetLocation( mBasicLightingEffect.GetUniformLocation( "uGlobalAmbientLight.Intensity" ) );

	mBasicLightingEffect.AddUniform( &mGlobalLight.Color );
	mBasicLightingEffect.AddUniform( &mGlobalLight.Intensity );

	delete[] basicVertTxt;
	delete[] basicFragTxt;
	
	// Generate the first level
	GenerateMap();
}
//---------------------------------------
Game::~Game()
{
	mWindow.Destroy();
}
//---------------------------------------
void Game::LoadAssets()
{
	// TODO put all the asset loading calls here...

	LoadKeys( "../data/Keys.xml" );
	LoadArea( "../data/TestDungeon.xml" );
//	LoadArea( "../data/BossLevel1.xml" );
	LoadPickups( "../data/Pickups.xml" );
	LoadEnemies( "../data/Enemies.xml" );
	LoadWeapons( "../data/Weapons.xml" );
}
//---------------------------------------
void Game::LoadArea( const char* filename )
{
	XmlReader reader( filename );

	// <Area>
	XmlReader::XmlReaderIterator itr = reader.ReadRoot();
	if ( itr.IsValid() )
	{
		const char* areaName = itr.GetAttributeAsCString( "name", "Dungeon" );
		int areaMaxRooms = itr.GetAttributeAsInt( "maxRooms", 0 );
		std::vector< int > minRoomSize, maxRoomSize, areaSize;
		itr.GetAttributeAsCSV( "minRoomSize", minRoomSize );
		itr.GetAttributeAsCSV( "maxRoomSize", maxRoomSize );
		itr.GetAttributeAsCSV( "areaSize", areaSize );
		float verticalness = itr.GetAttributeAsFloat( "verticalness", 0.5f );
		float verticalBiasUp = itr.GetAttributeAsFloat( "verticalness", 0.5f );
		float verticalBiasDown = itr.GetAttributeAsFloat( "verticalBiasDown", 0.5f );
		float doorChance = itr.GetAttributeAsFloat( "doorChance", 0.5f );
		float doorLockChance = itr.GetAttributeAsFloat( "doorLockChance", 0.5f );
		std::vector< float > ambientColor;
		itr.GetAttributeAsCSV( "ambientLightColor", ambientColor, "1,1,1" );
		float ambientIntensity = itr.GetAttributeAsFloat( "ambientLightIntensity", 1.0f );
		mEndDepth = itr.GetAttributeAsInt( "endDepth", 0 );
		mNextArea = itr.GetAttributeAsString( "nextArea", "" );

		if ( mNextArea.empty() )
			mEndDepth = 0;
		else
			mNextArea = "../data/" + mNextArea;

		mGenerator.Init( areaSize[0], areaSize[1], areaMaxRooms, minRoomSize[0], maxRoomSize[0], minRoomSize[1], maxRoomSize[1] );
		mGenerator.SetVerticalness( verticalness );
		mGenerator.SetVerticalBias( verticalBiasUp, verticalBiasDown );
		mGenerator.SetDoorChance( doorChance );
		mGenerator.SetDoorLockChance( doorLockChance );
		mGenerator.SetName( areaName );

		mGlobalLightColor = glm::vec3( ambientColor[0], ambientColor[1], ambientColor[2] );
		mGlobalLightIntensity = ambientIntensity;

		mGlobalLight.Color.SetValue( mGlobalLightColor );
		mGlobalLight.Intensity.SetValue( mGlobalLightIntensity );

		// Free old mappings of styles and object templates
		for ( auto i = mStyleMap.begin(); i != mStyleMap.end(); ++i )
		{
			delete i->second;
		}
		mStyleMap.clear();

		for ( auto i = mMapObjectMap.begin(); i != mMapObjectMap.end(); ++i )
		{
			delete i->second;
		}
		mMapObjectMap.clear();

		for ( auto i = mSpawnListMap.begin(); i != mSpawnListMap.end(); ++i )
		{
			delete i->second;
		}
		mSpawnListMap.clear();
		
		for ( auto i = mUseableObjects.begin(); i != mUseableObjects.end(); ++i )
		{
			delete *i;
		}
		mUseableObjects.clear();

		// <Styles>
		XmlReader::XmlReaderIterator styleslItr = itr.NextChild( "Styles" );
		if ( styleslItr.IsValid() )
		{
			std::string prefix = "../data/";
			for ( XmlReader::XmlReaderIterator styleItr = styleslItr.NextChild( "Style" );
				styleItr.IsValid(); styleItr = styleItr.NextSibling() )
			{
				TileStyle* style = new TileStyle;
				style->mUsageId = TileStyle::GetUsageIdFromString( styleItr.GetAttributeAsString( "usage", "none" ) );
				style->mMesh = Mesh::CreateMesh( prefix + styleItr.GetAttributeAsString( "file" ) );
				style->mName = styleItr.GetAttributeAsString( "name" );
				style->mCanBeLocked = styleItr.GetAttributeAsBool( "canBeLocked", true );
				style->mForceLocked = styleItr.GetAttributeAsBool( "forceLocked", false );
				style->LoadRulesFromXML( styleItr, &mGenerator, mStyleMap );
				style->LoadEventsFromXML( styleItr );
				mStyleMap[ style->mName ] = style;
			}
		}

		// <SpawnList>
		XmlReader::XmlReaderIterator spawnListsItr = itr.NextChild( "SpawnLists" );
		if ( spawnListsItr.IsValid() )
		{
			for ( XmlReader::XmlReaderIterator spawnListItr = spawnListsItr.NextChild( "SpawnList" );
				spawnListItr.IsValid(); spawnListItr = spawnListItr.NextSibling() )
			{
				std::string listName = spawnListItr.GetAttributeAsString( "name" );
				SpawnList* spawnList = new SpawnList;
				spawnListItr.GetAttributeAsCSV( "list", spawnList->mList );
				mSpawnListMap[ listName ] = spawnList;

				// Copy from another list
				std::vector< std::string > listToExtendFrom;
				spawnListItr.GetAttributeAsCSV( "extendsLists", listToExtendFrom );
				for ( auto listItr = listToExtendFrom.begin(); listItr != listToExtendFrom.end(); ++listItr )
				{
					SpawnList* list = mSpawnListMap[ *listItr ];
					if ( list )
					{
						spawnList->mList.insert( spawnList->mList.end(), list->mList.begin(), list->mList.end() );
					}
					else
					{
						WarnFail( "Could not find SpawnList of name '%s'. Did you define it after this SpawnList?\n", listItr->c_str() );
					}
				}
			}
		}

		// <Objects>
		XmlReader::XmlReaderIterator objectslItr = itr.NextChild( "Objects" );
		if ( objectslItr.IsValid() )
		{
			std::string prefix = "../data/";
			for ( XmlReader::XmlReaderIterator objectItr = objectslItr.NextChild( "Object" );
				objectItr.IsValid(); objectItr = objectItr.NextSibling() )
			{
				std::string name = objectItr.GetAttributeAsString( "name" );
				int usage = TileObject::GetUsageIdFromString( objectItr.GetAttributeAsString( "usage", "none" ) );
				TileObject* object;

				// Object specific properties
				if ( usage == TileObject::Usage_STATIC )
				{
					object = new TileObject();
					object->mMesh = Mesh::CreateMesh( prefix + objectItr.GetAttributeAsString( "file" ) );
				}
				else if ( usage == TileObject::Usage_PICKUP )
				{
					object = new TileObject_Pickup();
					object->mUsageId = usage;
					//object->mMesh = Mesh::CreateMesh( prefix + objectItr.GetAttributeAsString( "file" ) );

					TileObject_Pickup* pickupObject = (TileObject_Pickup*) object;
					pickupObject->mPickupName = objectItr.GetAttributeAsString( "pickupName" );
				}
				else if ( usage == TileObject::Usage_LIGHT )
				{
					object = new TileObject_Light();
					TileObject_Light* lightObject = (TileObject_Light*) object;

					std::vector< float > color;
					objectItr.GetAttributeAsCSV( "lightColor", color, "1,1,1" );
					if ( color.size() == 3 )
						lightObject->mLightColor = Color( color[0], color[1], color[2] );
					else
						DebugPrintf( "TileObject: invalid color - must be 'r,g,b'\n" );
					lightObject->mIntensity = objectItr.GetAttributeAsFloat( "lightIntensity", 0.5f );
					lightObject->mRadius = objectItr.GetAttributeAsFloat( "lightRadius", 1.0f );
					lightObject->mFalloff = objectItr.GetAttributeAsFloat( "lightFalloff", 0.5f );
				}
				else if ( usage == TileObject::Usage_ENEMY )
				{
					object = new TileObject_Enemy();
					TileObject_Enemy* enemyObject = (TileObject_Enemy*) object;
					enemyObject->mEnemyClass = objectItr.GetAttributeAsString( "enemyType" );
				}
				else if ( usage == TileObject::Usage_SPAWNER )
				{
					object = new TileObject_Spawner();
					TileObject_Spawner* spawner = (TileObject_Spawner*) object;
					std::string listName = objectItr.GetAttributeAsString( "spawnList" );
					auto listItr = mSpawnListMap.find( listName );
					if ( listItr != mSpawnListMap.end() )
					{
						spawner->mList = listItr->second;
					}
					else
					{
						WarnFail( "Cannot find SpawnList '%s'\n", listName.c_str() );
					}
				}

				// General properties
				object->mName = name;
				object->mUsageId = usage;
				object->mAttachment = mMapObjectMap[ objectItr.GetAttributeAsString( "attachment", "" ) ];
				object->LoadRulesFromXML( objectItr, &mGenerator, mMapObjectMap );
				object->LoadEventsFromXML( objectItr );
				std::vector< float > v;
				objectItr.GetAttributeAsCSV( "localSpawnOffset", v, "0,0,0" );
				if ( v.size() == 3 )
					object->mLocalSpawnOffset = glm::vec3( v[0], v[1], v[2] );
				else
					DebugPrintf( "TileObject: invalid local spawn offset - must be 'x,y,z'\n" );
				mMapObjectMap[ object->mName ] = object;
			}
		}


		// <Rooms>
		std::map< std::string, RoomTemplate* > roomMap;
		XmlReader::XmlReaderIterator roomTmplItr = itr.NextChild( "Rooms" );
		if ( roomTmplItr.IsValid() )
		{
			for ( XmlReader::XmlReaderIterator roomItr = roomTmplItr.NextChild( "Room" );
				roomItr.IsValid(); roomItr = roomItr.NextSibling() )
			{
				RoomTemplate& roomTmpl = mGenerator.CreateRoomTemplate();
				const char* name = roomItr.GetAttributeAsCString( "name", 0 );
				// Rooms are only mapped if they are named
				if ( name )
				{
					roomMap[ name ] = &roomTmpl;
				}
				// Load rules
				roomTmpl.LoadRulesFromXML( roomItr, &mGenerator, roomMap );
				// Copy base rooms
				std::vector< std::string > roomsToCopyFrom;
				roomItr.GetAttributeAsCSV( "extendsRooms", roomsToCopyFrom );
				for ( auto roomCopyItr = roomsToCopyFrom.begin(); roomCopyItr != roomsToCopyFrom.end(); ++roomCopyItr )
				{
					RoomTemplate* roomToCopy = roomMap[ *roomCopyItr ];
					if ( roomToCopy )
					{
						roomTmpl.CopyDataFrom( roomToCopy );
					}
					else
					{
						WarnFail( "Could not find room of name '%s'. Did you define it after this room?\n", roomCopyItr->c_str() );
					}
				}

				if ( roomItr.HasAttribute( "minRoomSize" ) )
				{
					std::vector< int > minRoomSize;
					roomItr.GetAttributeAsCSV( "minRoomSize", minRoomSize );
					roomTmpl.SetMinSize( minRoomSize[0], minRoomSize[1] );
				}
				if ( roomItr.HasAttribute( "maxRoomSize" ) )
				{
					std::vector< int > maxRoomSize;
					roomItr.GetAttributeAsCSV( "maxRoomSize", maxRoomSize );
					roomTmpl.SetMaxSize( maxRoomSize[0], maxRoomSize[1] );
				}

				std::map< std::string, Useable* > usableMap;
				std::map< std::string, Useable* > usableStyleMap;
				for ( XmlReader::XmlReaderIterator roomJtr = roomItr.NextChild();
					roomJtr.IsValid(); roomJtr = roomJtr.NextSibling() )
				{
					if ( roomJtr.ElementNameEquals( "UseStyle" ) )
					{
						std::string styleName = roomJtr.GetAttributeAsString( "uses" );
						Useable* useStyle = new Useable;
						mUseableObjects.push_back( useStyle );
						useStyle->mObject = mStyleMap[ styleName ];
						useStyle->LoadRulesFromXML( roomJtr, &mGenerator, usableStyleMap );
						
						roomTmpl.AddStyle( useStyle );

						const char* name = roomJtr.GetAttributeAsCString( "name", 0 );
						// Useables are only mapped if they are named
						if ( name )
						{
							usableStyleMap[ name ] = useStyle;
						}	
					}
					else if ( roomJtr.ElementNameEquals( "UsesObject" ) )
					{
						std::string objectName = roomJtr.GetAttributeAsString( "uses" );
						auto objItr = mMapObjectMap.find( objectName );
						if ( objItr == mMapObjectMap.end() )
						{
							WarnFail( "<Room name='%s'> <UsesObject uses='%s'>: No object named '%s'\n", name, objectName.c_str(), objectName.c_str() );
							continue;
						}
						Useable* useObject = new Useable;
						mUseableObjects.push_back( useObject );
						useObject->mObject = mMapObjectMap[ objectName ];
						useObject->LoadRulesFromXML( roomJtr, &mGenerator, usableMap );
						roomTmpl.AddObject( useObject );

						const char* name = roomJtr.GetAttributeAsCString( "name", 0 );
						// Useables are only mapped if they are named
						if ( name )
						{
							usableMap[ name ] = useObject;
						}
					}
				}
			}
		}
	}
	else
	{
		WarnFail( "Failed to open '%s'\n", filename );
	}
}
//---------------------------------------
void Game::LoadKeys( const char* filename )
{
	XmlReader reader( filename );

	// <Keys>
	XmlReader::XmlReaderIterator itr = reader.ReadRoot();
	if ( itr.IsValid() )
	{
		std::string rootDir = "../data/";
		std::string defaultPrefix = itr.GetAttributeAsString( "defaultPrefix", "Key" );
		std::string defaultIconName = itr.GetAttributeAsString( "defaultIcon" );
		std::string defaultModel = itr.GetAttributeAsString( "defaultModel" );
		for ( XmlReader::XmlReaderIterator keyItr = itr.NextChild( "Key" );
			  keyItr.IsValid(); keyItr = keyItr.NextSibling( "Key" ) )
		{
			std::string prefix = keyItr.GetAttributeAsString( "prefix", defaultPrefix );
			std::string suffix = keyItr.GetAttributeAsString( "suffix" );
			std::string iconName = keyItr.GetAttributeAsString( "icon", defaultIconName );
			std::string modelName = keyItr.GetAttributeAsString( "model", defaultModel );
			Texture2D* icon = Texture2D::CreateTexture( rootDir + iconName );
			Mesh* mesh = Mesh::CreateMesh( rootDir + modelName );
			Key::KeyStyle style( prefix + " " + suffix, icon, mesh );
			Key::AddKeyStyle( style );
		}
	}
	else
	{
		WarnFail( "Failed to open '%s'\n", filename );
	}
}
//---------------------------------------
void Game::LoadPickups( const char* filename )
{
	XmlReader reader( filename );

	// <Pickups>
	XmlReader::XmlReaderIterator itr = reader.ReadRoot();
	if ( itr.IsValid() )
	{
		Pickup::LoadPickupTempaltesFromXml( itr );
	}
	else
	{
		WarnFail( "Failed to open '%s'\n", filename );
	}
}
//---------------------------------------
void Game::LoadEnemies( const char* filename )
{
	XmlReader reader( filename );

	// <Enemies>
	XmlReader::XmlReaderIterator itr = reader.ReadRoot();
	if ( itr.IsValid() )
	{
		Enemy::LoadEnemyTempaltesFromXml( itr );
	}
	else
	{
		WarnFail( "Failed to open '%s'\n", filename );
	}
}
//---------------------------------------
void Game::LoadWeapons( const char* filename )
{
	XmlReader reader( filename );

	// <Weapons>
	XmlReader::XmlReaderIterator itr = reader.ReadRoot();
	if ( itr.IsValid() )
	{
		Weapon::LoadWeaponTempaltesFromXml( itr );
	}
	else
	{
		WarnFail( "Failed to open '%s'\n", filename );
	}
}
//---------------------------------------
void Game::Run()
{
	mRunning = true;
	float dt = 1.0f / 60.0f;

	const int FRAMES_TO_SAMPLE = 10;
	unsigned frameTimes[FRAMES_TO_SAMPLE];
	unsigned lastTime;
	unsigned frameCount;
	float framesPerSec;

	memset( frameTimes, 0, sizeof( frameTimes ) );
	frameCount = 0;
	framesPerSec = 0;
	lastTime = SDL_GetTicks();

	while ( mRunning )
	{
		// Check if exit reached
		glm::vec3 p1 = mPlayer.GetPosition();
		glm::vec3 p2 = mGenerator.GetExitLocation();
		float dist = glm::distance( p1, p2 );
		if ( dist < 0.5f )
		{
			GenerateMap();
		}

		HandleEvents();

		//
		// Updates
		//

		// Entities
		for ( auto itr = mEntities.begin(); itr != mEntities.end(); ++itr )
			(*itr)->Update( dt );

		// Remove dead
		RemoveDeadEntities();

		// Step physics
		mPhysicsWorld.Step( dt );

		// Player
		mPlayer.Update( dt );
		if ( mPlayerLight )
			mPlayerLight->mPosition = mCamera.position;

		// GameLog
		GameLog::Instance.Update( dt );

		//
		// Rendering
		//

		// Begin Drawing
		mWindow.Prepare();

		// Prepare the lighting
		EnableMostRelevantLights();

		// Entities - Scene
		mWindow.SetActiveEffect( &mBasicLightingEffect );
		mWindow.SetMatrixMode( Window::MATRIX_MODE_PROJECTION );
		mWindow.LoadMatrix( mCamera.projectionMatrix );
		mWindow.SetMatrixMode( Window::MATRIX_MODE_MODEL );
		mWindow.LoadMatrix( mCamera.viewMatrix );
		for ( auto itr = mEntitiesSceneGroup.begin(); itr != mEntitiesSceneGroup.end(); ++itr )
			(*itr)->Draw( &mWindow );
		mWindow.SetActiveEffect( 0 );

		
		// Debug Visuals
		//
		Texture2D::Unbind();
		// World axes
		mWindow.DrawWorldAxes( (float) mGenerator.GetWidth(), 10.0f, (float) mGenerator.GetHeight() );
		// Physics debug
		mPhysicsWorld.DebugDraw();
		// Map - Debug Draw
		mGenerator.Draw( &mWindow );

		// Entities - Foreground
		mWindow.SetActiveEffect( &mBasicLightingEffect );
		mWindow.SetMatrixMode( Window::MATRIX_MODE_PROJECTION );
		mWindow.LoadMatrix( mCamera.projectionMatrix );
		mWindow.SetMatrixMode( Window::MATRIX_MODE_MODEL );
		mWindow.LoadMatrix( mCamera.viewMatrix );
		mWindow.ClearDepth();
		for ( auto itr = mEntitiesForegroundGroup.begin(); itr != mEntitiesForegroundGroup.end(); ++itr )
			(*itr)->Draw( &mWindow );
		mWindow.SetActiveEffect( 0 );


		// 2D Drawing
		//
		if ( !mHideHUD )
		{
			mWindow.DrawDebugText( 0, 200.0f, Color::WHITE, 0.75f, mGenerator.GetFloorName() );

			// Minimap
			mMinimap.SetCenter( mCamera.position.x, mCamera.position.z, glm::degrees( mCamera.horizontalAngle ) );
			mMinimap.UpdateVisibility( 50.0f );
			mMinimap.Draw( &mWindow );

			// GameLog
			GameLog::Instance.Draw( &mWindow );

			// HUD
			mWindow.DrawDebugTextFmt( 0, mWindow.GetHeight() * 0.9f - 24.0f, Color::WHITE, "AR: %d%%", (int) ( 100.0f * mPlayer.GetCurrentArmor() / (float) mPlayer.GetMaxArmor() ) );
			mWindow.DrawDebugTextFmt( 0, mWindow.GetHeight() * 0.9f, Color::WHITE, "HP: %d%%", (int) ( 100.0f * mPlayer.GetCurrentLife() / (float) mPlayer.GetMaxLife() ) );
			mWindow.DrawDebugTextFmt( mWindow.GetWidth() - 250.0f, 0, Color::WHITE, "FPS %.3f", framesPerSec );

			// Draw player inventory
			mPlayer.Draw( &mWindow );
		}
		// Help
		if ( mShowHelp )
		{
			mWindow.DrawDebugTextFmt( mWindow.GetWidth() * 0.25f, mWindow.GetHeight() * 0.25f, Color::WHITE,
				"F1    Toggle Help\n" \
				"F2    Free/Lock Mouse\n" \
				"F3    Toggle Ghost\n" \
				"F4    Toggle Collision Debug\n" \
				"F5    Generate Random Level\n" \
				"F6    Reload Data Files\n" \
				"F7    Toggle Connectivity Debug\n" \
				"F8    Reveal minimap\n" \
				"F10  Toggle HUD\n" \
				"L      Toggle Bright Lighting\n" \
				"TAB  Toggle Fullscreen Map\n"
			);
		}

		// Nice fade effect after loading
		if ( mLoadFadeInTime > 0 )
		{
			mLoadFadeInTime -= dt;
			Color c = Color::BLACK;
			c.a = mLoadFadeInTime;
			Texture2D::Unbind();
			mWindow.SetDrawColor( c );
			mWindow.SetOrtho();
			mWindow.DrawQuad2D( 0, 0, (float) mWindow.GetWidth(), (float) mWindow.GetHeight() );
			mWindow.SetPerspective();
		}

		// Show everything
		mWindow.Present();

		// FPS
		unsigned frameIndex;
		unsigned tick;
		unsigned count;

		frameIndex = frameCount % FRAMES_TO_SAMPLE;
		tick = SDL_GetTicks();
		frameTimes[frameIndex] = tick - lastTime;
		lastTime = tick;
		++frameCount;
		if ( frameCount < FRAMES_TO_SAMPLE )
			count = frameCount;
		else
			count = FRAMES_TO_SAMPLE;
		framesPerSec = 0;
		for ( unsigned i = 0; i < count; ++i )
		{
			framesPerSec += frameTimes[i];
		}
		framesPerSec /= count;
		framesPerSec = 1000.0f / framesPerSec;
	}
}
//---------------------------------------
void Game::GenerateMap()
{
	mWindow.Prepare();
	mWindow.DrawDebugText( mWindow.GetWidth() * 0.01f, mWindow.GetHeight() * 0.95f, Color::WHITE, "Loading..." );
	mWindow.Present();
	mLoadFadeInTime = 0.75f;

	if ( mEndDepth > 0 && mGenerator.GetCurrentDepth() == mEndDepth )
	{
		LoadArea( mNextArea.c_str() );
	}

	GameLog::Instance.Clear();

	// Destroy old entities
	DestroyEntities();

	// Destroy lights
	DestroyLights();

	Mesh::InvalidatePhysics();

	// Re-create the physics world and add player
	mPhysicsWorld.DestroyPhysics();
	mPhysicsWorld.InitPhysics();

	// Initialize lighting
	InitializeLights();
	
	// Generate a new map
	mGenerator.Generate();

	// Player setup
	mPlayerLight = CreateLight( Color::WHITE, 0.25f, 0.2f, 0.5f );
	mPlayer.SetCamera( &mCamera );
	mPlayer.InitPhysics( &mPhysicsWorld );
	mPlayer.Teleport( mGenerator.GetStartLocation() );
	mPlayer.ClearInventory();
	mPlayer.InitializeWeapons();

	// Spawn entities
	mGenerator.SpawnEntities( this );

	GameLog::Instance.PostMessageFmt( "Now entering %s", mGenerator.GetFloorName() );
}
//---------------------------------------
void Game::HandleEvents()
{
	SDL_Event sdlEvent;
	static int mx;
	static int my;
	static bool lockmouse = false;
	static bool lightingEnabled = true;
	bool left = false, right = false, forward = false, backward = false, up = false, down = false, sprint = false;

	while ( SDL_PollEvent( &sdlEvent ) )
	{
		switch ( sdlEvent.type )
		{
		case SDL_KEYDOWN:
			{
				if ( sdlEvent.key.keysym.sym == SDLK_ESCAPE )
				{
					mRunning = false;
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_w )
				{
					mPlayer.SetForward( 1 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_s )
				{
					mPlayer.SetBackward( 1 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_a )
				{
					mPlayer.SetStrafeLeft( 1 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_d )
				{
					mPlayer.SetStrafeRight( 1 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_e )
				{
					mPlayer.UseAction();
					break;
				}
				break;
			}
		case SDL_KEYUP:
			{
				if ( sdlEvent.key.keysym.sym == SDLK_F1 )
				{
					mShowHelp = !mShowHelp;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F2 )
				{
					lockmouse = !lockmouse;
					if ( lockmouse )
						SDL_ShowCursor( 1 );
					else
						SDL_ShowCursor( 0 );
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F3 )
				{
					mCamera.ghost = !mCamera.ghost;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F4 )
				{
					mPhysicsWorld.ShowDebugCollision = !mPhysicsWorld.ShowDebugCollision;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F5 )
				{
					GenerateMap();
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F6 )
				{
					LoadAssets();
					GenerateMap();
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F7 )
				{
					mGenerator.DebugSectors = !mGenerator.DebugSectors;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F8 )
				{
					mMinimap.RevealAll();
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_F10 )
				{
					mHideHUD = !mHideHUD;
					mPlayer.HideHUD( mHideHUD );
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_l )
				{
					lightingEnabled = !lightingEnabled;
					if ( lightingEnabled )
					{
						mGlobalLight.Color.SetValue( mGlobalLightColor );
						mGlobalLight.Intensity.SetValue( mGlobalLightIntensity );
					}
					else
					{
						mGlobalLight.Color.SetValue( glm::vec3( 1.0f ) );
						mGlobalLight.Intensity.SetValue( 1.0f );
					}
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_w )
				{
					mPlayer.SetForward( 0 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_s )
				{
					mPlayer.SetBackward( 0 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_a )
				{
					mPlayer.SetStrafeLeft( 0 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_d )
				{
					mPlayer.SetStrafeRight( 0 );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_TAB )
				{
					mMinimapIsFullscreen = !mMinimapIsFullscreen;
					if ( mMinimapIsFullscreen )
						mMinimap.SetSize( (float) mWindow.GetWidth(), (float) mWindow.GetHeight() );
					else
						mMinimap.SetSize( mWindow.GetWidth() * 0.15f, mWindow.GetWidth() * 0.15f );
					break;
				}
				else if ( sdlEvent.key.keysym.sym == SDLK_q )
				{
					//GameLog::Instance.PostMessage( "test message" );
					break;
				}
				break;
			}
		case SDL_MOUSEMOTION:
			{
				mx = sdlEvent.motion.x;
				my = sdlEvent.motion.y;
				break;
			}
		case SDL_MOUSEBUTTONDOWN:
			{
				if ( sdlEvent.button.button == SDL_BUTTON_LEFT )
					mPlayer.StartFire();
				else if ( sdlEvent.button.button == SDL_BUTTON_WHEELUP )
					mPlayer.NextWeapon();
				else if ( sdlEvent.button.button == SDL_BUTTON_WHEELDOWN )
					mPlayer.PrevWeapon();
				break;
			}
		case SDL_MOUSEBUTTONUP:
			{
				if ( sdlEvent.button.button == SDL_BUTTON_LEFT )
					mPlayer.StopFire();
				break;
			}
		case SDL_ACTIVEEVENT:
			{
				mHasFocus = sdlEvent.active.gain == 1;
				break;
			}
		case SDL_QUIT:
			mRunning = false;
			break;
		}
	}

	if ( mHasFocus )
	{
		const Uint8 *state = SDL_GetKeyState(NULL);
		left = state[SDLK_a] != 0;
		right = state[SDLK_d] != 0;
		forward = state[SDLK_w] != 0;
		backward = state[SDLK_s] != 0;
		up = state[SDLK_e] != 0;
		down = state[SDLK_q] != 0;
		sprint = state[SDLK_LSHIFT] != 0;

		if ( !lockmouse )
		{
			// Compute new orientation
			mPlayer.Rotate( mCamera.mouseSpeed * float( mWindow.GetWidth() / 2 - mx ) );
			mCamera.ComputeFromInput( 0.0166f, left, right, forward, backward, up, down, sprint, mx, my );
		}
	}
}
//---------------------------------------
void Game::AddEntity( Entity* entity )
{
	entity->LoadAssets();
	entity->Initialize();
	entity->InitPhysics( &mPhysicsWorld );

	mEntities.push_back( entity );
	// Keep entities sorted
	std::sort( mEntities.begin(), mEntities.end(), []( Entity* A, Entity* B )
	{
		return *A < *B;
	});

	if ( entity->GetRenderGroup() == Entity::RG_SCENE )
		mEntitiesSceneGroup.push_back( entity );
	else if ( entity->GetRenderGroup() == Entity::RG_FOREGROUND )
		mEntitiesForegroundGroup.push_back( entity );
	else
	{
		WarnCrit( "Entity with no RenderGroup!\n" );
		assert( 0 );
	}
}
//---------------------------------------
void Game::DestroyEntities()
{
	for ( auto itr = mEntities.begin(); itr != mEntities.end(); ++itr )
		delete *itr;
	mEntities.clear();
	mEntitiesSceneGroup.clear();
	mEntitiesForegroundGroup.clear();
}
//---------------------------------------
PointLight* Game::CreateLight( const Color& color, float intensity, float falloff, float radius )
{
	PointLight* light = new PointLight();
	mPointLights.push_back( light );
	light->mColor = color;
	light->mIntensity = intensity;
	light->mFalloff = falloff;
	light->mRadius = radius;
	return light;
}
//---------------------------------------
void Game::InitializeLights()
{
	for ( int i = 0; i < MAX_LIGHT_COUNT; ++i )
	{
		mGLPointLights.push_back( new GLPointLight() );
		GLPointLight* light = mGLPointLights.back();
		int lightIndex = mGLPointLights.size() - 1;

		std::stringstream colorName, intensityName, constAttenName, linearAttenName, posName;
		colorName       << "uPointLights[" << lightIndex << "].Color";
		intensityName   << "uPointLights[" << lightIndex << "].Intensity";
		constAttenName  << "uPointLights[" << lightIndex << "].ConstantAtten";
		linearAttenName << "uPointLights[" << lightIndex << "].LinearAtten";
		posName         << "uPointLights[" << lightIndex << "].Position";

		light->Color.SetLocation( mBasicLightingEffect.GetUniformLocation( colorName.str().c_str() ) );
		light->Intensity.SetLocation( mBasicLightingEffect.GetUniformLocation( intensityName.str().c_str() ) );
		light->ConstantAtten.SetLocation( mBasicLightingEffect.GetUniformLocation( constAttenName.str().c_str() ) );
		light->LinearAtten.SetLocation( mBasicLightingEffect.GetUniformLocation( linearAttenName.str().c_str() ) );
		light->Position.SetLocation( mBasicLightingEffect.GetUniformLocation( posName.str().c_str() ) );

		mBasicLightingEffect.AddUniform( &light->Color );
		mBasicLightingEffect.AddUniform( &light->Intensity );
		mBasicLightingEffect.AddUniform( &light->ConstantAtten );
		mBasicLightingEffect.AddUniform( &light->LinearAtten );
		mBasicLightingEffect.AddUniform( &light->Position );

		// Avoid div 0 in shader
		light->ConstantAtten.SetValue( 1.0f );
	}
}
//---------------------------------------
void Game::DestroyLights()
{
	for ( auto itr = mGLPointLights.begin(); itr != mGLPointLights.end(); ++itr )
	{
		GLPointLight*& light = *itr;

		mBasicLightingEffect.RemoveUniform( &light->Color );
		mBasicLightingEffect.RemoveUniform( &light->Intensity );
		mBasicLightingEffect.RemoveUniform( &light->ConstantAtten );
		mBasicLightingEffect.RemoveUniform( &light->LinearAtten );
		mBasicLightingEffect.RemoveUniform( &light->Position );

		delete light;
	}
	mGLPointLights.clear();
	mPlayerLight = 0;

	//for ( auto itr = mPointLights.begin(); itr != mPointLights.end(); ++itr )
	//{
	//	delete *itr;
	//}
	mPointLights.clear();
}
//---------------------------------------
void Game::DestroyLight( PointLight* light )
{
	auto itr = std::find( mPointLights.begin(), mPointLights.end(), light );
	if ( itr != mPointLights.end() )
	{
		mPointLights.erase( itr );
//		delete light;
//		light = 0;
	}
}
//---------------------------------------
bool Game::IsRelevant( Entity* obj ) const
{
	const glm::vec3& objPos = obj->GetPosition();
	glm::vec3 center;
	if ( mCamera.ghost )
		center = mCamera.position;
	else
		center = mPlayer.GetPosition();
	const float d = glm::distance( objPos, center );
	return d <= MAX_RELEVANT_DISTANCE;
}
//---------------------------------------
void Game::EnableMostRelevantLights()
{
	std::sort( mPointLights.begin(), mPointLights.end(), [&]( PointLight* a, PointLight* b ) -> bool
	{
		const glm::vec3& aPos = a->GetPosition();
		const glm::vec3& bPos = b->GetPosition();
		glm::vec3 center;
		if ( mCamera.ghost )
			center = mCamera.position;
		else
			center = mPlayer.GetPosition();
		const float d1 = glm::distance( aPos, center ) - a->mRadius;
		const float d2 = glm::distance( bPos, center ) - b->mRadius;
		return d1 < d2;
	});

	for ( int i = 0; i < MAX_LIGHT_COUNT; ++i )
	{
		if ( i < (int) mPointLights.size() )
		{
			GLPointLight* glLight = mGLPointLights[i];
			PointLight* light = mPointLights[i];

			glLight->Color.SetValue( glm::vec3( light->mColor.r, light->mColor.g, light->mColor.b ) );
			glLight->ConstantAtten.SetValue( 1.0f - light->mFalloff );
			glLight->LinearAtten.SetValue( light->mRadius );
			glLight->Intensity.SetValue( light->mIntensity );
			glLight->Position.SetValue( light->GetPosition() );
		}
		else if ( i < MAX_LIGHT_COUNT )
		{
			GLPointLight* glLight = mGLPointLights[i];
			glLight->Intensity.SetValue( 0.0f );
		}
	}
}
//---------------------------------------
void Game::RemoveDeadEntities()
{
	// Remove from render groups
	mEntitiesSceneGroup.erase( 
		std::remove_if( mEntitiesSceneGroup.begin(), mEntitiesSceneGroup.end(), []( Entity* e ) -> bool
	{
		const bool b = e->WantsDeletion();
		return b;
	}), mEntitiesSceneGroup.end() );

	mEntitiesForegroundGroup.erase( 
		std::remove_if( mEntitiesForegroundGroup.begin(), mEntitiesForegroundGroup.end(), []( Entity* e ) -> bool
	{
		const bool b = e->WantsDeletion();
		return b;
	}), mEntitiesForegroundGroup.end() );

	// Delete from scene
	mEntities.erase( 
		std::remove_if( mEntities.begin(), mEntities.end(), []( Entity* e ) -> bool
	{
		const bool b = e->WantsDeletion();
		if ( b )
			delete e;
		return b;
	}), mEntities.end() );
}
//---------------------------------------