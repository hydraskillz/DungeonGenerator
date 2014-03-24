#include "MapTile.h"
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
MapTile::MapTile( Tile* tile )
	: Object()
	, mTile( tile )
{
	mMesh = mTile->mStyle->mMesh;
}
//---------------------------------------
MapTile::~MapTile()
{}
//---------------------------------------
void MapTile::LoadAssets()
{
}
//---------------------------------------
void MapTile::InitPhysics( PhysicsWorld* world )
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
void MapTile::Draw( Window* window )
{
	if ( !mVisible )
		return;

	if ( !Game::Get()->IsRelevant( this ) )
		return;
	
	float m[16];
	mXForm.getOpenGLMatrix( m );

	Texture2D::Unbind();
	window->SetDrawColor( Color::WHITE );

	glm::mat4 mat = glm::make_mat4( m );

	window->PushMatrix();
	window->MultMatrix( mat );
	window->BeginDraw();
	mMesh->Draw();
	window->PopMatrix();

	/*glPushMatrix();
	glMultMatrixf( m );
	mMesh->Draw();
	glPopMatrix();*/
}
//---------------------------------------