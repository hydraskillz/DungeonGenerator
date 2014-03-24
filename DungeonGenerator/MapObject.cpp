#include "MapObject.h"
#include "DungeonGenerator.h"
#include "Mesh.h"
#include "PhysicsWorld.h"
#include "Window.h"
#include "Texture.h"
#include "Util.h"
#include "Logger.h"
#include "Game.h"

#include <glew.h>
#include <glm/gtc/type_ptr.hpp>

//---------------------------------------
MapObject::MapObject( Tile* tile, Mesh* mesh )
	: mTile( tile )
	, mMesh( mesh )
	, mColor( Color::WHITE )
{}
//---------------------------------------
MapObject::~MapObject()
{}
//---------------------------------------
void MapObject::LoadAssets()
{
}
//---------------------------------------
void MapObject::InitPhysics( PhysicsWorld* world )
{
	btCollisionShape* shape = mMesh->GetCollisionShape();
	if ( !shape )
		shape = world->GenerateTriangleMeshCollision( mMesh );
	if ( shape )
	{
		mBody = world->AddStaticMesh( shape );
		mBody->setUserPointer( &Entity::WORLD );
		SetLocation( glm::vec3( mTile->x, mTile->z, mTile->y ) );
		SetRotation( glm::vec3( 0, 1.0f, 0 ), glm::radians( mTile->GetOrientation() ) );
	}
	else
	{
		DebugPrintf( "No valid collision for Mesh '%s'\n", mMesh->GetName() );
	}
}
//---------------------------------------
void MapObject::Draw( Window* window )
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