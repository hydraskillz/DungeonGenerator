#include "Enemy.h"
#include "Game.h"
#include "Texture.h"
#include "Window.h"
#include "MD2Model.h"
#include "MD2Animation.h"
#include "Util.h"
#include "EntityFactory.h"

#include <glew.h>
#include <glm/gtc/type_ptr.hpp>

//---------------------------------------
class EnemyTemplate
	: public EntityTemplateBase
{
public:
	EnemyTemplate( const XmlReader::XmlReaderIterator& xmlItr )
		: EntityTemplateBase( xmlItr )
		, mBody( 0 )
		, mHead( 0 )
		, mWeapon( 0 )
		, mSpecialDeath( 0 )
	{
		const std::string dataRoot = "../data/";
		const std::string bodyPath = xmlItr.GetAttributeAsString( "bodyModel", "" );
		const std::string headPath = xmlItr.GetAttributeAsString( "headModel", "" );
		const std::string weaponPath = xmlItr.GetAttributeAsString( "weaponModel", "" );
		const std::string deathPath = xmlItr.GetAttributeAsString( "deathModel", "" );

		mDeathAnimName = xmlItr.GetAttributeAsString( "deathAnimName", "" );
		mIdleAnimName = xmlItr.GetAttributeAsString( "idleAnimName", "" );
		mPainAnimName = xmlItr.GetAttributeAsString( "painAnimName", "" );
		mAttackAnimName = xmlItr.GetAttributeAsString( "attackAnimName", "" );

		mBodyTextureIndex = xmlItr.GetAttributeAsInt( "bodyTextureIndex", 0 );
		mHeadTextureIndex = xmlItr.GetAttributeAsInt( "headTextureIndex", 0 );

		if ( !bodyPath.empty() )
			mBody = MD2Model::CreateMD2Model( dataRoot + bodyPath );
		if ( !headPath.empty() )
			mHead = MD2Model::CreateMD2Model( dataRoot + headPath );
		if ( !weaponPath.empty() )
			mWeapon = MD2Model::CreateMD2Model( dataRoot + weaponPath );
		if ( !deathPath.empty() )
			mSpecialDeath = MD2Model::CreateMD2Model( dataRoot + deathPath );
	}

	MD2Model* mBody;
	MD2Model* mHead;
	MD2Model* mWeapon;
	MD2Model* mSpecialDeath;
	int mBodyTextureIndex;
	int mHeadTextureIndex;
	std::string mDeathAnimName;
	std::string mIdleAnimName;
	std::string mPainAnimName;
	std::string mAttackAnimName;
};
//---------------------------------------
class EnemyFactory
	: public EntityFactoryBase< Enemy, EnemyTemplate >
{
public:
	void InternalCreateEntity( Enemy& entity, const EnemyTemplate& entityTmpl )
	{
		if ( entityTmpl.mBody )
			entity.mBodyAnim = new MD2Animation( entityTmpl.mBody, entityTmpl.mBodyTextureIndex );
		if ( entityTmpl.mHead )
			entity.mHeadAnim = new MD2Animation( entityTmpl.mHead, entityTmpl.mHeadTextureIndex );
		if ( entityTmpl.mWeapon )
			entity.mWeaponAnim = new MD2Animation( entityTmpl.mWeapon );
		if ( entityTmpl.mSpecialDeath )
			entity.mSpecialDeathAnim = new MD2Animation( entityTmpl.mSpecialDeath );

		entity.mTemplate = &entityTmpl;
	}
};
//---------------------------------------
void Enemy::LoadEnemyTempaltesFromXml( const XmlReader::XmlReaderIterator& xmlItr )
{
	Entity::LoadTemplatesFromXml< Enemy >( xmlItr );
}
//---------------------------------------
Enemy* Enemy::CreateEnemy( const std::string& templateName )
{
	return Entity::CreateEntity< Enemy >( templateName );
}
//---------------------------------------
EnemyFactory* Enemy::GetFactory()
{
	static EnemyFactory factory;
	return &factory;
}
//---------------------------------------


//---------------------------------------
Enemy::Enemy()
	: Actor()
	, mBodyAnim( 0 )
	, mHeadAnim( 0 )
	, mWeaponAnim( 0 )
	, mGibs( 0 )
	, mSpecialDeathAnim( 0 )
	, mGibed( false )
{}
//---------------------------------------
Enemy::~Enemy()
{
	SafeDelete( mBodyAnim );
	SafeDelete( mHeadAnim );
	SafeDelete( mWeaponAnim );
	SafeDelete( mGibs );
	SafeDelete( mSpecialDeathAnim );
}
//---------------------------------------
void Enemy::Initialize()
{
	if ( mBodyAnim )
		mBodyAnim->SetOnCompleteListener( this );
	
	if ( mBodyAnim ) mBodyAnim->PlayAnim( mTemplate->mIdleAnimName );
	if ( mHeadAnim ) mHeadAnim->PlayAnim( mTemplate->mIdleAnimName );
	if ( mWeaponAnim ) mWeaponAnim->PlayAnim( mTemplate->mIdleAnimName );
}
//---------------------------------------
void Enemy::LoadAssets()
{
	//mGibs = new MD2Animation( MD2Model::CreateMD2Model( "../data/gibs.md2" ) );
	//mGibs->SetPlayRate( 20 );
}
//---------------------------------------
void Enemy::Update( float dt )
{
	if ( mBodyAnim )
		mBodyAnim->Update( dt );
	if ( mHeadAnim )
		mHeadAnim->Update( dt );
	if ( mWeaponAnim )
		mWeaponAnim->Update( dt );
	if ( !IsAlive() )
	{
		if ( mGibs )
			mGibs->Update( dt );
		if ( mSpecialDeathAnim )
			mSpecialDeathAnim->Update( dt );
	}
}
//---------------------------------------
void Enemy::Draw( Window* window )
{
	if ( !Game::Get()->IsRelevant( this ) )
		return;

	window->SetDrawColor( Color::WHITE );

	float m[16];
	mGhostObject->getWorldTransform().getOpenGLMatrix( m );
	glm::mat4 mat = glm::make_mat4( m );
	mat[3][1] -= 0.28f;

	window->PushMatrix();
	window->MultMatrix( mat );
	window->Scale( 0.01f, 0.01f, 0.01f );

	if ( mGibed && mGibs )
		mGibs->Draw( window );
	else
	{
		if ( !IsAlive() && mSpecialDeathAnim )
		{
			if ( mSpecialDeathAnim )
				mSpecialDeathAnim->Draw( window );
		}
		else
		{
			if ( mBodyAnim )
				mBodyAnim->Draw( window );
			if ( mHeadAnim )
				mHeadAnim->Draw( window );
			if ( mWeaponAnim )
				mWeaponAnim->Draw( window );
		}
	}

	window->PopMatrix();
}
//---------------------------------------
void Enemy::PrepareFire()
{
	if ( IsAlive() )
	{
		if ( mBodyAnim ) mBodyAnim->PlayAnim( mTemplate->mAttackAnimName, false );
		if ( mHeadAnim ) mHeadAnim->PlayAnim( mTemplate->mAttackAnimName, false );
		if ( mWeaponAnim ) mWeaponAnim->PlayAnim( mTemplate->mAttackAnimName, false );
	}
}
//---------------------------------------
void Enemy::TakeDamage( int damage, Entity* instigator )
{
	Hurt( damage );

	if ( IsAlive() && mBodyAnim && !mBodyAnim->IsAnimationPlaying( mTemplate->mPainAnimName.c_str() ) )
	{
		Face( instigator );

		if ( mBodyAnim ) mBodyAnim->PlayAnim( mTemplate->mPainAnimName, false );
		if ( mHeadAnim ) mHeadAnim->PlayAnim( mTemplate->mPainAnimName, false );
		if ( mWeaponAnim ) mWeaponAnim->PlayAnim( mTemplate->mPainAnimName, false );
	}
}
//---------------------------------------
void Enemy::Kill()
{
	Entity::Kill();

	if ( mBodyAnim ) mBodyAnim->PlayAnim( mTemplate->mDeathAnimName, false );
	if ( mHeadAnim ) mHeadAnim->PlayAnim( mTemplate->mDeathAnimName, false );
	SafeDelete( mWeaponAnim );

	if ( mSpecialDeathAnim )
		mSpecialDeathAnim->PlayAnim( mTemplate->mDeathAnimName, false );

	if ( mGibed && mGibs )
		mGibs->PlayAnim( "splt0", false );
}
//---------------------------------------
void Enemy::OnAnimationComplete( const char* animName )
{
	if ( !strcmp( animName, mTemplate->mPainAnimName.c_str() ) )
	{
		PrepareFire();
	}
}
//---------------------------------------