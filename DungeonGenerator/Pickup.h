/*
 * Author      : Matthew Johnson
 * Date        : 5/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Object.h"
#include "Color.h"
#include "Ammo.h"
#include "XmlReader.h"
#include <map>

class Player;
class Mesh;
class PickupFactory;
class PickupTemplate;

class Pickup
	: public Object
{
	friend class Entity;
	friend class PickupFactory;
public:
	static void LoadPickupTempaltesFromXml( const XmlReader::XmlReaderIterator& xmlItr );
	static Pickup* CreatePickup( const std::string& templateName );

	Pickup();
	virtual ~Pickup();

	virtual void InitPhysics( PhysicsWorld* world );
	virtual void Draw( Window* window );
	virtual void Update( float dt );

	// Called by the actor who picked this up
	virtual void OnPickup( Player* actor );

	void SetColor( const Color& color ) { mColor = color; }
	const Color& GetColor() { return mColor; }

	void SetMesh( Mesh* mesh ) { mMesh = mesh; }

protected:
	float mSpinnerTimer;
	Color mColor;
	Mesh* mMesh;
	const PickupTemplate* mTempalte;

private:
	static PickupFactory* GetFactory();
};