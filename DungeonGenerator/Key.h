/*
 * Author      : Matthew Johnson
 * Date        : 4/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Pickup.h"

class PointLight;

class Key
	: public Pickup
{
public:
	struct KeyStyle
	{
		KeyStyle()
			: name( "Key" )
			, icon( 0 )
		{}

		KeyStyle( const std::string& name, Texture2D* icon, Mesh* mesh )
			: name( name )
			, icon( icon )
			, mesh( mesh )
		{}

		std::string name;
		Texture2D* icon;
		Mesh* mesh;
	};
	static void AddKeyStyle( const KeyStyle& style );
	static void ClearKeyStyles();
	static void RandomizeKeyStyles();
	static const KeyStyle& GetRandomKeyStyle();

	Key( int keyId );
	virtual ~Key();

	void Initialize();
	void LoadAssets();
	void InitPhysics( PhysicsWorld* world );
	void Draw( Window* window );
	void Update( float dt );

	void OnPickup( Player* actor );
	
	int GetKeyId() const { return mKeyId; }
	void SetKeyStyle( const KeyStyle& name ) { mKeyStyle = name; }
	const KeyStyle& GetKeyStyle() const { return mKeyStyle; }
	const std::string& GetKeyName() const { return mKeyStyle.name; }

private:
	KeyStyle mKeyStyle;
	int mKeyId;		// Used to check which door this key opens
	PointLight* mKeyLight;

	static std::vector< KeyStyle > mKeyStyles;
	static unsigned mNextKeyName;
};