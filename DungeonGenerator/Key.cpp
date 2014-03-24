#include "Key.h"
#include "Util.h"
#include "Window.h"
#include "PhysicsWorld.h"
#include "Actor.h"
#include "Inventory.h"
#include "Texture.h"
#include "Mesh.h"
#include "GameLog.h"
#include "Game.h"
#include "Logger.h"

#include <glew.h>
#include <glm/gtc/type_ptr.hpp>

std::vector< Key::KeyStyle > Key::mKeyStyles;
unsigned Key::mNextKeyName = 0;

//---------------------------------------
void Key::AddKeyStyle( const KeyStyle& style )
{
	mKeyStyles.push_back( style );
}
//---------------------------------------
void Key::ClearKeyStyles()
{
	mKeyStyles.clear();
}
//---------------------------------------
void Key::RandomizeKeyStyles()
{
	std::random_shuffle( mKeyStyles.begin(), mKeyStyles.end() );
}
//---------------------------------------
const Key::KeyStyle& Key::GetRandomKeyStyle()
{
	static const KeyStyle DUMMY_STYLE;
	if ( mKeyStyles.size() == 0 )
		return DUMMY_STYLE;
	return mKeyStyles[ mNextKeyName++ % mKeyStyles.size() ];
}
//---------------------------------------


//---------------------------------------
Key::Key( int keyId )
	: Pickup()
	, mKeyId( keyId )
	, mKeyLight( 0 )
{}
//---------------------------------------
Key::~Key()
{}
//---------------------------------------
void Key::Initialize()
{
	mKeyLight = Game::Get()->CreateLight( mColor, 0.9f, 0.7f, 1.5f );
	mKeyLight->mPosition = mPosition;
	Game::Get()->AddEntity( mKeyLight );
	AddAttachment( mKeyLight );
}
//---------------------------------------
void Key::LoadAssets()
{
	//mMesh = Mesh::CreateMesh( "../data/obj/KeyCard.obj" );
	mMesh = mKeyStyle.mesh;
}
//---------------------------------------
void Key::InitPhysics( PhysicsWorld* world )
{
	Object::InitPhysics( world );

	mBody = world->GenerateBoxCollision( mMesh, true ); //AddBox( glm::vec3( 1, 1, 1 ), true );
	Translate( glm::vec3( 0, -0.25f, 0 ) );
	mBody->setWorldTransform( mXForm );
	//mBody->getCollisionShape()->setLocalScaling( btVector3( 0.15f, 0.15f, 0.15f ) );
	mBody->setUserPointer( this );
}
//---------------------------------------
void Key::Draw( Window* window )
{
	if ( !mVisible )
		return;

	if ( !Game::Get()->IsRelevant( this ) )
		return;

	float m[16];
	mXForm.getOpenGLMatrix( m );

	Texture2D::Unbind();
	window->SetDrawColor( mColor );

	glm::mat4 mat = glm::make_mat4( m );

	window->PushMatrix();
	window->MultMatrix( mat );
	window->BeginDraw();
	mMesh->Draw();
	window->PopMatrix();
}
//---------------------------------------
void Key::Update( float dt )
{
	Pickup::Update( dt );

	mKeyLight->mPosition = mPosition;
}
//---------------------------------------
void Key::OnPickup( Player* actor )
{
	if ( IsAlive() )
	{
		DebugPrintf( "Got key %d\n", mKeyId );
		GameLog::Instance.PostMessageFmt( "Got %s\n", mKeyStyle.name.c_str() );

		// Give item
		InventoryItem* item = new InventoryItem( InventoryItem::ITEM_KEY, mKeyId );
		item->mTexture = mKeyStyle.icon;//Texture2D::CreateTexture( "../data/textures/key_icon.png" );
		item->mColor = mColor;
		actor->AddItemToInventory( item );
		// Kill the key so it can't be picked up twice
		Kill();
		// Destroy the pickup in the world
		Destroy();
	}
}
//---------------------------------------