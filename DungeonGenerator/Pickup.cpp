#include "Pickup.h"
#include "Actor.h"
#include "Window.h"
#include "Mesh.h"
#include "PhysicsWorld.h"
#include "Player.h"
#include "Game.h"
#include "GameLog.h"
#include "Logger.h"
#include "EntityFactory.h"
#include "Weapon.h"

#include <glew.h>
#include <glm/gtc/type_ptr.hpp>

//---------------------------------------
class PickupTemplate
	: public EntityTemplateBase
{
public:
	PickupTemplate::PickupTemplate( const XmlReader::XmlReaderIterator& xmlItr )
		: EntityTemplateBase( xmlItr )
	{
		mMesh = Mesh::CreateMesh( "../data/" + xmlItr.GetAttributeAsString( "model" ) );

		mLifeMod = 0;
		mArmorMod = 0;
		memset( mAmmoMod, 0, sizeof( mAmmoMod ) );

		for ( XmlReader::XmlReaderIterator itr = xmlItr.NextChild( "ModStat" );
			itr.IsValid(); itr = itr.NextSibling() )
		{
			std::string statName = itr.GetAttributeAsString( "name" );
			int statValue = itr.GetAttributeAsInt( "value" );

			if ( statName == "life" )
				mLifeMod += statValue;
			else if ( statName == "armor" )
				mArmorMod += statValue;
			else if ( statName == "ammo-bullets" )
				mAmmoMod[ AMMO_BULLET ] += statValue;
		}

		for ( XmlReader::XmlReaderIterator itr = xmlItr.NextChild( "GiveWeapon" );
			itr.IsValid(); itr = itr.NextSibling() )
		{
			mWeaponsToGive.push_back( itr.GetAttributeAsString( "name" ) );
		}
	}

	Mesh* mMesh;
	int mLifeMod;
	int mArmorMod;
	int mAmmoMod[ AMMO_TYPE_COUNT ];
	std::vector< std::string > mWeaponsToGive;
};
//---------------------------------------
class PickupFactory
	: public EntityFactoryBase< Pickup, PickupTemplate >
{
protected:
	void InternalCreateEntity( Pickup& entity, const PickupTemplate& entityTmpl )
	{
		entity.mMesh = entityTmpl.mMesh;
		entity.mTempalte = &entityTmpl;
	}
};
//---------------------------------------
PickupFactory* Pickup::GetFactory()
{
	static PickupFactory factory;
	return &factory;
}
//---------------------------------------

//---------------------------------------
void Pickup::LoadPickupTempaltesFromXml( const XmlReader::XmlReaderIterator& xmlItr )
{
	Entity::LoadTemplatesFromXml< Pickup >( xmlItr );
}
//---------------------------------------
Pickup* Pickup::CreatePickup( const std::string& templateName )
{
	return Entity::CreateEntity< Pickup >( templateName );
}
//---------------------------------------


//---------------------------------------
Pickup::Pickup()
	: mMesh( 0 )
	, mSpinnerTimer( 0 )
	, mTempalte( 0 )
{
	mEntityType = ET_PICKUP;
	mColor = Color::WHITE;
}
//---------------------------------------
Pickup::~Pickup()
{}
//---------------------------------------
void Pickup::InitPhysics( PhysicsWorld* world )
{
	Object::InitPhysics( world );

	mBody = world->AddBox( glm::vec3( 1, 1, 1 ), true );
	mBody->setWorldTransform( mXForm );
	mBody->getCollisionShape()->setLocalScaling( btVector3( 0.15f, 0.15f, 0.15f ) );
	mBody->setUserPointer( this );
}
//---------------------------------------
void Pickup::Draw( Window* window )
{
	if ( !mVisible || !mMesh )
		return;
	if ( !Game::Get()->IsRelevant( this ) )
		return;

	float m[16];
	mXForm.getOpenGLMatrix( m );

//	Texture2D::Unbind();
//	window->SetDrawColor( mColor );

	glm::mat4 mat = glm::make_mat4( m );
	window->SetDrawColor( mColor );
	window->PushMatrix();
	window->MultMatrix( mat );
	window->Scale( mTempalte->mScale, mTempalte->mScale, mTempalte->mScale );
	window->BeginDraw();
	mMesh->Draw();
	window->PopMatrix();
}
//---------------------------------------
void Pickup::Update( float dt )
{
	mSpinnerTimer += 3.5f * dt;

	Translate( glm::vec3( 0, 0.0025f * std::cos( mSpinnerTimer ), 0 ) );
	SetRotation( glm::vec3( 0, 1, 0 ), mSpinnerTimer );
}
//---------------------------------------
void Pickup::OnPickup( Player* actor )
{
	bool wasPickedUp = false;

	if ( mTempalte )
	{
		if ( actor->AddLife( mTempalte->mLifeMod ) )
		{
			wasPickedUp = true;
			GameLog::Instance.PostMessageFmt( "Got %d Life", mTempalte->mLifeMod );
		}

		if ( actor->AddArmor( mTempalte->mArmorMod ) )
		{
			wasPickedUp = true;
			GameLog::Instance.PostMessageFmt( "Got %d Armor", mTempalte->mArmorMod );
		}

		for ( int i = 0; i < AMMO_TYPE_COUNT; ++i )
		{
			if ( actor->AddAmmo( mTempalte->mAmmoMod[ i ], i ) )
			{
				wasPickedUp = true;
				GameLog::Instance.PostMessageFmt( "Got %d %s", mTempalte->mAmmoMod[ i ], gAmmoNames[ i ] );
			}
		}

		for ( auto itr = mTempalte->mWeaponsToGive.begin(); itr != mTempalte->mWeaponsToGive.end(); ++itr )
		{
			if ( !actor->GiveWeapon( Weapon::CreateWeapon( *itr ) ) )
			{
				// TODO give correct amount of ammo
				actor->AddAmmo( 10, 0 );
			}
			else
			{
				GameLog::Instance.PostMessageFmt( "Got the %s", itr->c_str() );
			}
			wasPickedUp = true;
		}
	}

	if ( wasPickedUp )
	{
		Kill();
		Destroy();
	}
}
//---------------------------------------