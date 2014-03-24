/*
 * Author      : Matthew Johnson
 * Date        : 23/Jan/2014
 * Description :
 *   Handles drawing to the active window.
 */
 
#pragma once

#include "Color.h"

#include <glm/glm.hpp>
#include <stack>

class Texture2D;
class BitmapFont;
class Effect;
class Camera;

class Window
{
public:
	Window();
	~Window();

	enum
	{
		MATRIX_MODE_MODEL,
		MATRIX_MODE_PROJECTION
	};

	bool Init();
	void Destroy();
	void Prepare();
	void Present();

	void SetOrtho();
	void SetPerspective();

	int GetHeight() const { return mHeight; }
	int GetWidth() const { return mWidth; }

	void SetDrawColor( const Color& color );
	void DrawWorldAxes( float x, float y, float z );

	void DrawQuad2D( float x, float y, float w, float h );
	void DrawQuad( float x, float y, float z, float w, float h );
	void DrawTriangle2D( float x, float y, float w, float h, float a );
	void DrawRect( Texture2D* texture, float x, float y, const Color& color, int cx, int cy, int cw, int ch );
	void DrawDebugText( float x, float y, const Color& color, const char* txt );
	void DrawDebugText( float x, float y, const Color& color, float scale, const char* txt );
	void DrawDebugTextFmt( float x, float y, const Color& color, const char* fmt, ... );

	void SetScissorRegion( int x, int y, int w, int h );
	void EnableScissor();
	void DisableScissor();

	void SetActiveEffect( Effect* effect );

	void PushMatrix();
	void PopMatrix();
	void MultMatrix( const glm::mat4& mat );
	void LoadMatrix( const glm::mat4& mat );
	void LoadIdentity();
	void SetMatrixMode( int mode );
	void Scale( float x, float y, float z );
	void Rotate( float a, float x, float y, float z );
	void Translate( float x, float y, float z );
	void SetCamera( Camera* camera ) { mCamera = camera; }
	void SetDepthTest( bool enbale );
	void ClearDepth();

	// Sends the current matrices to the active shader
	// interpFactor will blend between two bound VBOs (vertex animation)
	void BeginDraw( float interpFactor=0.0f );

private:
	int mWidth, mHeight;
	BitmapFont* mDebugFont;
	Effect* mActiveEffect;
	Camera* mCamera;
	glm::mat4 mActiveMatrix[2];
	std::stack< glm::mat4 > mMatrixStack[2];
	int mCurrentMatrixMode;
	int mMVPLocation;
	int mModelLocation;
	int mColorLocation;
	int mInterpFactorLocation;
};