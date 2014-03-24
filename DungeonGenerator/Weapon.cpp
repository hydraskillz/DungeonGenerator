#include "Weapon.h"
#include "Window.h"
#include "Player.h"
#include "Util.h"
#include "glm/gtc/quaternion.hpp"
#include "Game.h"
#include "Decal.h"
#include "Logger.h"
#include "Mesh.h"
#include "PhysicsWorld.h"
#include "EntityFactory.h"
#include <glm/gtc/type_ptr.hpp>

//---------------------------------------
class WeaponTemplate
	: public EntityTemplateBase
{
public:
	WeaponTemplate::WeaponTemplate( const XmlReader::XmlReaderIterator& xmlItr )
		: EntityTemplateBase( xmlItr )
	{
		mMesh = Mesh::CreateMesh( "../data/" + xmlItr.GetAttributeAsString( "model" ) );
	}

	Mesh* mMesh;
};
//---------------------------------------
class WeaponFactory
	: public EntityFactoryBase< Weapon, WeaponTemplate >
{
protected:
	void InternalCreateEntity( Weapon& entity, const WeaponTemplate& entityTmpl )
	{
		entity.mName = entityTmpl.mName;
		entity.mWeaponMesh = entityTmpl.mMesh;
		entity.mTempalte = &entityTmpl;
	}
};
//---------------------------------------
WeaponFactory* Weapon::GetFactory()
{
	static WeaponFactory factory;
	return &factory;
}
//---------------------------------------


//---------------------------------------
void Weapon::LoadWeaponTempaltesFromXml( const XmlReader::XmlReaderIterator& xmlItr )
{
	Entity::LoadTemplatesFromXml< Weapon >( xmlItr );
}
//---------------------------------------
Weapon* Weapon::CreateWeapon( const std::string& templateName )
{
	return Entity::CreateEntity< Weapon >( templateName );
}
//---------------------------------------


//---------------------------------------
Weapon::Weapon()
	: mIsFiring( false )
	, mStopFire( true )
	, mFireTimer( 0 )
{
	mEntityType = ET_WEAPON;
	mRenderGroup = RG_FOREGROUND;
}
//---------------------------------------
Weapon::~Weapon()
{}
//---------------------------------------
void Weapon::LoadAssets()
{
//	mWeaponMesh = Mesh::CreateMesh( "../data/models/weapons/BFG9000/BFG9000.obj" );
//	mWeaponMesh = Mesh::CreateMesh( "../data/models/weapons/shotgun/shotgun.obj" );
}
//---------------------------------------
void Weapon::Update( float dt )
{
	if ( mIsFiring )
	{
		mFireTimer += dt;
		if ( mFireTimer > 0.2f )
		{
			mFireTimer = 0;
			mIsFiring = !mStopFire;
			DoFire();
		}
	}

	/*for ( int i = 0; i < mDecals.size(); ++i )
	{
		mDecals[i].life -= dt;
	}
	mDecals.erase( 
	std::remove_if( mDecals.begin(), mDecals.end(), []( const Decal& d )
	{
		return d.life <= 0.0f;
	}),mDecals.end() );*/
}
//---------------------------------------
void Weapon::Draw( Window* window )
{
	if ( !mVisible || !mWeaponMesh )
		return;

	static float a=1.57f, b, c;
	static int i = -1;
	float bobX = 0, bobY = 0;
	if ( mIsFiring )
	{
		c += 1.4f;

		bobX = std::sin( c );
		bobY = 0.5f * std::cos( c );
	}
	else if ( mOwner->IsMoving() )
	{
		a += i*0.2f;
		if ( ( i == -1 && a < -1.571f ) || a > 1.57f )
			i = -i;

		bobX = 0.5f * std::sin( a );
		bobY = 0.5f * std::cos( a );
	}
	else
	{
		b += 0.05f;

	//	bobX = 2 * std::sin( b );
		bobY = 0.5f * std::cos( b );
	}

	window->SetMatrixMode( Window::MATRIX_MODE_MODEL );
	window->PushMatrix();
	window->LoadIdentity();

	window->Scale( mTempalte->mScale, mTempalte->mScale, mTempalte->mScale );
	window->Rotate( mTempalte->mRotate[0], mTempalte->mRotate[1], mTempalte->mRotate[2], mTempalte->mRotate[3] );
	window->Translate( bobX + mTempalte->mTranslate[0], bobY + mTempalte->mTranslate[1], mTempalte->mTranslate[2] );
	
	window->BeginDraw();

	mWeaponMesh->Draw();

	window->PopMatrix();
}
//---------------------------------------
void Weapon::StartFire()
{
	mIsFiring = true;
	mStopFire = false;
}
//---------------------------------------
void Weapon::StopFire()
{
	mStopFire = true;
}
//---------------------------------------
void Weapon::DoFire()
{
	glm::vec3 hitNorm;
	glm::vec3 hitLoc;
	Entity* hit = mOwner->Raycast( hitNorm, hitLoc );
	if ( hit )
	{

		hit->TakeDamage( 15, mOwner );
		/*
		Decal* d = new Decal();
		d->LoadAssets();
	//	d->Attach( hit );

	//	mDecals.push_back( Decal() );
	//	mDecals.back().pos = hitLoc;
	//	mDecals.back().norm = hitNorm;

		glm::vec3 up( 0, 1, 0 );
		glm::vec3 axis = glm::cross( up, hitNorm );
		if ( glm::dot( axis, axis ) < 0.001f )
			axis = glm::cross( hitNorm, glm::vec3( 1, 0, 0 ) );
		float angle = std::atan2( (float)axis.length(), glm::dot( up, hitNorm ) );
		glm::normalize( axis );
		glm::vec3 axis2 = glm::cross( hitNorm, axis );
		glm::normalize( axis2 );
		
		float s = 0.025f;
		glm::mat4 m;
		glm::mat4 sm(
			s, 0, 0, 0,
			0, s, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );
		m[0] = glm::vec4( axis, 0 );
		m[1] = glm::vec4( axis2, 0 );
		m[2] = glm::vec4( hitNorm, 0 );
		if ( hit == &Entity::WORLD )
			m[3] = glm::vec4( hitLoc + 0.0025f * hitNorm, 1 );
		else
			m[3] = glm::vec4( 0.025f * hitNorm, 1 );
		m *= sm;

		d->SetTransform( m );
		
	//	mDecals.back().m = m;
	//	mDecals.back().life = 10.0f;

		Game::Get()->AddEntity( d );

		DebugPrintf( "Weapon: Hit[ %.3f %.3f %.3f ] N[ %.3f %.3f %.3f ] AX[ %.3f %.3f %.3f ] A[ %.3f ]\n"
			, hitLoc.x, hitLoc.y, hitLoc.z
			, hitNorm.x, hitNorm.y, hitNorm.z
			, axis.x, axis.y ,axis.z
			, angle );*/
	}
}
//---------------------------------------