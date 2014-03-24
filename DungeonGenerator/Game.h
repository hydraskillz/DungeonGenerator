/*
 * Author      : Matthew Johnson
 * Date        : 16/Jan/2014
 * Description :
 *   
 */
 
#pragma once

#include "DungeonGenerator.h"
#include "Window.h"
#include "Camera.h"
#include "PhysicsWorld.h"
#include "Player.h"
#include "Minimap.h"
#include "Effect.h"
#include "Uniform.h"
#include "PointLight.h"

#include <glm/glm.hpp>

class Object;

class Game
{
public:
	static Game* Get();
	// This value must match MAX_POINT_LIGHTS in the light shader
	static const int MAX_LIGHT_COUNT = 64;
	// Max distance an object can be before it is not drawn
	static const float MAX_RELEVANT_DISTANCE;

	Game();
	~Game();

	void Run();

	// Add an Entity to the game
	// Entities will have their initialization functions called and be updated/draw
	void AddEntity( Entity* entity );

	// Create a new light in the level
	// You still need to add the light to the scene
	PointLight* CreateLight( const Color& color, float intensity, float falloff, float radius );

	// Remove a light previously created by CreateLight
	// Does not delete or remove the light from the scene
	void DestroyLight( PointLight* light );

	// Check if an object is close enough to the player to be relevant for drawing
	bool IsRelevant( Entity* obj ) const;

private:
	// Mark the nearest lights to be sent to the GPU
	void EnableMostRelevantLights();

	void GenerateMap();

	void HandleEvents();

	void LoadArea( const char* filename );
	void LoadKeys( const char* filename );
	void LoadPickups( const char* filename );
	void LoadEnemies( const char* filename );
	void LoadWeapons( const char* filename );

	void LoadAssets();
	void DestroyEntities();
	void InitializeLights();
	void DestroyLights();
	void RemoveDeadEntities();

	static Game* mInstance;
	DungeonGenerator mGenerator;
	Window mWindow;
	bool mRunning;
	bool mMinimapIsFullscreen;
	bool mShowHelp;
	bool mHideHUD;
	bool mHasFocus;
	float mLoadFadeInTime;
	int mEndDepth;
	std::string mNextArea;

	Camera mCamera;

	Player mPlayer;
	Minimap mMinimap;

	std::vector< Entity* > mEntities;
	std::vector< Entity* > mEntitiesSceneGroup;
	std::vector< Entity* > mEntitiesForegroundGroup;

	// Generation
	std::map< std::string, TileStyle* > mStyleMap;
	std::map< std::string, TileObject* > mMapObjectMap;
	std::map< std::string, SpawnList* > mSpawnListMap;
	std::vector< Useable* > mUseableObjects;

	// Lighting
	Effect mBasicLightingEffect;
	
	struct GlobalAmbientLight
	{
		Uniform3f Color;
		Uniform1f Intensity;
	};
	GlobalAmbientLight mGlobalLight;
	float mGlobalLightIntensity;
	glm::vec3 mGlobalLightColor;

	struct GLPointLight
	{
		Uniform3f Color;
		Uniform1f Intensity;
		Uniform3f Position;
		Uniform1f ConstantAtten;
		Uniform1f LinearAtten;
	};

	std::vector< GLPointLight* > mGLPointLights;
	std::vector< PointLight* > mPointLights;
	PointLight* mPlayerLight;	// Weak pointer into mPointLights

	// Physics
	PhysicsWorld mPhysicsWorld;
};