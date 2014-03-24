#include "Decal.h"
#include "Window.h"
#include "Texture.h"

//---------------------------------------
Decal::Decal( float life )
	: mLife( life )
{}
//---------------------------------------
Decal::~Decal()
{}
//---------------------------------------
void Decal::LoadAssets()
{
	mDecalTexture = Texture2D::CreateTexture( "../data/textures/BULLETDECAL1.png" );
}
//---------------------------------------
void Decal::Update( float dt )
{
	mLife -= dt;
	if ( mLife < 0 )
		Kill();
}
//---------------------------------------
void Decal::Draw( Window* window )
{
	/*mDecalTexture->Bind();
	Color c = Color::WHITE;
	if ( mLife <= 1.0f )
		c.a = mLife / 1.0f;
	window->SetDrawColor( c );
	glm::mat4 m = mTransform;
	if ( mParent )
		m *= mParent->GetTransform();
	window->DrawQuad( m );*/
}
//---------------------------------------