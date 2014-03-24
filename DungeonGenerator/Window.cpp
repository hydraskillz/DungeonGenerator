#include "Window.h"
#include "Texture.h"
#include "Mesh.h"
#include "BitmapFont.h"
#include "Effect.h"
#include "Camera.h"
#include "MD2Model.h"

#include <SDL.h>
#include <glew.h>
#include <glm/gtc/matrix_transform.hpp>

SDL_Surface* gWindow;

//---------------------------------------
Window::Window()
	: mCamera( 0 )
{}
//---------------------------------------
Window::~Window()
{}
//---------------------------------------
bool Window::Init()
{
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		SDL_Quit();
		return false;
	}

	mWidth = 1280;
	mHeight = 720;

	gWindow = SDL_SetVideoMode( mWidth, mHeight, 32, SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE );
	SDL_WM_SetCaption( "Dungeon Generator", 0 );
	SDL_ShowCursor( 0 );
	SDL_WarpMouse( mWidth / 2, mHeight / 2 );
	
	glewInit();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CW );

	glClearColor( 0, 0, 0, 1 );
	glViewport( 0, 0, mWidth, mHeight );
	
	glMatrixMode( GL_PROJECTION );
	gluPerspective( 45.0f, (float) ( mWidth ) / mHeight, 0.1f,  1000.0f);

	glMatrixMode( GL_MODELVIEW );

	mDebugFont = new BitmapFont( "../data/fonts/default.fnt" );

	return true;
}
//---------------------------------------
void Window::Destroy()
{
	Texture2D::DestroyAllTextures();
	MD2Model::DestroyAllMD2Models();
	Mesh::DestroyAllMeshes();
	delete mDebugFont;

	SDL_FreeSurface( gWindow );
	SDL_Quit();
}
//---------------------------------------
void Window::Prepare()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( &mCamera->viewMatrix[0][0] );
}
//---------------------------------------
void Window::Present()
{
	SDL_GL_SwapBuffers();
}
//---------------------------------------
void Window::SetOrtho()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glViewport( 0, 0, mWidth, mHeight );
	glOrtho( 0, mWidth, mHeight, 0, 1, -1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glDisable( GL_DEPTH_TEST );
}
//---------------------------------------
void Window::SetPerspective()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, (float) ( mWidth ) / mHeight, 0.1f,  1000.0f);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glEnable( GL_DEPTH_TEST );
}
//---------------------------------------
void Window::SetDrawColor( const Color& color )
{
	if ( mActiveEffect )
	{
		glUniform3fv( mColorLocation, 1, &color.r );
	}
	else
	{
		glColor4fv( &color.r );
	}
}
//---------------------------------------
void Window::DrawWorldAxes( float x, float y, float z )
{
	glDisable( GL_TEXTURE_2D );

	glBegin( GL_LINES );
	glColor3f( 1, 0, 0 );
	glVertex3f( 0, 0, 0 );
	glVertex3f( x, 0, 0 );
	glColor3f( 0, 1, 0 );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, y, 0 );
	glColor3f( 0, 0, 1 );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, 0, z );
	glEnd();

	glEnable( GL_TEXTURE_2D );
}
//---------------------------------------
void Window::DrawQuad2D( float x, float y, float w, float h )
{
	glPushMatrix();
	glTranslatef( x, y, 0 );
	glScalef( w, h, 1 );

	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0 );
	glVertex2f( 0, 0 );
	glTexCoord2f( 1.0f, 0 );
	glVertex2f( 1.0f, 0 );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f( 1.0f, 1.0f );
	glTexCoord2f( 0, 1.0f );
	glVertex2f( 0, 1.0f );
	glEnd();
	glPopMatrix();
}
//---------------------------------------
void Window::DrawQuad( float x, float y, float z, float w, float h )
{
	float cx, cy, cz;
	float hw, hh;

	hw = w / 2.0f;
	hh = h / 2.0f;
	cx = (float) x;
	cy = (float) y;
	cz = (float) z;

	float nx = 0;
	float ny = 1.0f;
	float nz = 0;

	glPushMatrix();
	glTranslatef( cx, cz, cy );
	
	glScalef( w, h, 1.0f );
	glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 );
		glNormal3f( 0, 1.0f, 0 );
		glVertex3f( -0.5f, 0, -0.5f );

		glTexCoord2f( 1.0f, 0 );
		glNormal3f( 0, 1.0f, 0 );
		glVertex3f( 0.5f, 0, -0.5f );

		glTexCoord2f( 1.0f, 1.0f );
		glNormal3f( 0, 1.0f, 0 );
		glVertex3f( 0.5f, 0, 0.5f );

		glTexCoord2f( 0, 1.0f );
		glNormal3f( 0, 1.0f, 0 );
		glVertex3f( -0.5f, 0, 0.5f );
	glEnd();
	glPopMatrix();
}
//---------------------------------------
void Window::DrawTriangle2D( float x, float y, float w, float h, float a )
{
	float hw = w / 2.0f;
	float hh = h / 2.0f;

	glPushMatrix();
	glTranslatef( x, y, 0 );
	glRotatef( a, 0, 0, -1.0f );
	
	glBegin( GL_TRIANGLES );
		glVertex2f( 0, h );
		glVertex2f( -hw, -hh );
		glVertex2f( hw, -hh );
	glEnd();
	glPopMatrix();
}
//---------------------------------------
void Window::DrawRect( Texture2D* texture, float x, float y, const Color& color, int cx, int cy, int cw, int ch )
{
	glPushMatrix();
	glTranslatef( x, y, 0 );
	glScalef( (float) cw, (float) ch, 1 );

	texture->Bind();
	SetDrawColor( color );

	float u = (float) ( cx ) / texture->GetWidth();
	float v = (float) ( cy ) / texture->GetHeight();
	float s = (float) ( cw ) / texture->GetWidth();
	float t = (float) ( ch ) / texture->GetHeight();

	glBegin( GL_QUADS );
	glTexCoord2f( u, v );
	glVertex2f( 0, 0 );
	glTexCoord2f( u + s, v );
	glVertex2f( 1, 0 );
	glTexCoord2f( u + s, v + t );
	glVertex2f( 1, 1 );
	glTexCoord2f( u, v + t );
	glVertex2f( 0, 1 );
	glEnd();
	glPopMatrix();
}
//---------------------------------------
void Window::DrawDebugText( float x, float y, const Color& color, float scale, const char* txt )
{
	mDebugFont->RenderText( this, x, y, txt, color, scale );
}
//---------------------------------------
void Window::DrawDebugText( float x, float y, const Color& color, const char* txt )
{
	mDebugFont->RenderText( this, x, y, txt, color );
}
//---------------------------------------
void Window::DrawDebugTextFmt( float x, float y, const Color& color, const char* fmt, ... )
{
	char textFormatBuffer[ 1024 ];

	// Apply text formating
	va_list vargs;
	va_start( vargs, fmt );
	vsprintf_s( textFormatBuffer, fmt, vargs );
	va_end( vargs );

	mDebugFont->RenderText( this, x, y, textFormatBuffer, color );
}
//---------------------------------------
void Window::SetScissorRegion( int x, int y, int w, int h )
{
	glScissor( x, y + mHeight - h, w, h );
}
//---------------------------------------
void Window::EnableScissor()
{
	glEnable( GL_SCISSOR_TEST );
}
//---------------------------------------
void Window::DisableScissor()
{
	glDisable( GL_SCISSOR_TEST );
}
//---------------------------------------
void Window::SetActiveEffect( Effect* effect )
{
	mActiveEffect = effect;
	if ( mActiveEffect == 0 )
	{
		Effect::Disable();
	}
	else
	{
		mActiveEffect->Apply();
		mMVPLocation = mActiveEffect->GetUniformLocation( "uMVP" );
		mModelLocation = mActiveEffect->GetUniformLocation( "uModel" );
		mColorLocation = mActiveEffect->GetUniformLocation( "uColor" );
		mInterpFactorLocation = mActiveEffect->GetUniformLocation( "uInterpolationFactor" );
	}
}
//---------------------------------------
void Window::PushMatrix()
{
	mMatrixStack[ mCurrentMatrixMode ].push( mActiveMatrix[ mCurrentMatrixMode ] );
}
//---------------------------------------
void Window::PopMatrix()
{
	if ( !mMatrixStack[ mCurrentMatrixMode ].empty() )
	{
		mActiveMatrix[ mCurrentMatrixMode ] = mMatrixStack[ mCurrentMatrixMode ].top();
		mMatrixStack[ mCurrentMatrixMode ].pop();
	}
}
//---------------------------------------
void Window::MultMatrix( const glm::mat4& mat )
{
	mActiveMatrix[ mCurrentMatrixMode ] *= mat;
}
//---------------------------------------
void Window::LoadMatrix( const glm::mat4& mat )
{
	mActiveMatrix[ mCurrentMatrixMode ] = mat;
}
//---------------------------------------
void Window::LoadIdentity()
{
	mActiveMatrix[ mCurrentMatrixMode ] = glm::mat4( 1 );
}
//---------------------------------------
void Window::SetMatrixMode( int mode )
{
	mCurrentMatrixMode = mode;
}
//---------------------------------------
void Window::Scale( float x, float y, float z )
{
	mActiveMatrix[ mCurrentMatrixMode ] = glm::scale( mActiveMatrix[ mCurrentMatrixMode ], glm::vec3( x, y, z ) );
}
//---------------------------------------
void Window::Rotate( float a, float x, float y, float z )
{
	mActiveMatrix[ mCurrentMatrixMode ] = glm::rotate( mActiveMatrix[ mCurrentMatrixMode ], a, glm::vec3( x, y, z ) );
}
//---------------------------------------
void Window::Translate( float x, float y, float z )
{
	mActiveMatrix[ mCurrentMatrixMode ] = glm::translate( mActiveMatrix[ mCurrentMatrixMode ], glm::vec3( x, y, z ) );
}
//---------------------------------------
void Window::BeginDraw( float interpFactor )
{
	glm::mat4 mvp = mActiveMatrix[ MATRIX_MODE_PROJECTION ] * mActiveMatrix[ MATRIX_MODE_MODEL ];
	glm::mat4 m = glm::inverse( mCamera->viewMatrix ) * mActiveMatrix[ MATRIX_MODE_MODEL ];
	glUniformMatrix4fv( mMVPLocation, 1, GL_FALSE, &mvp[0][0] );
	glUniformMatrix4fv( mModelLocation, 1, GL_FALSE, &m[0][0] );
	glUniform1f( mInterpFactorLocation, interpFactor );
}
//---------------------------------------
void Window::SetDepthTest( bool enbale )
{
	if ( enbale )
		glEnable( GL_DEPTH_TEST );
	else
		glDisable( GL_DEPTH_TEST );
}
//---------------------------------------
void Window::ClearDepth()
{
	glClear( GL_DEPTH_BUFFER_BIT );
}
//---------------------------------------