#include "MD2Animation.h"
#include "MD2Model.h"
#include "Window.h"

//---------------------------------------
MD2Animation::MD2Animation( MD2Model* model, int textureIndex )
	: mModel( model )
	, mOnCompleteCB( 0 )
	, mCurrentAnimName( 0 )
	, mCurrentFrame( 0 )
	, mNextFrame( 0 )
	, mStartFrame( 0 )
	, mEndFrame( 0 )
	, mRate( 10 )
	, mInterpTime( 0.0f )
	, mTextureIndex( textureIndex )
	, mIsPlaying( false )
{}
//---------------------------------------
MD2Animation::~MD2Animation()
{}
//---------------------------------------
void MD2Animation::Update( float dt )
{
	if ( mIsPlaying )
	{
		mInterpTime += dt * mRate;

		if ( mInterpTime > 1.0f )
		{
			mInterpTime = 0.0f;

			++mCurrentFrame;
			++mNextFrame;

			if ( mCurrentFrame > mEndFrame )
				if ( mIsLooping )
					mCurrentFrame = mStartFrame;
				else
				{
					mIsPlaying = false;
					mCurrentFrame = mEndFrame;
					if ( mOnCompleteCB )
						mOnCompleteCB->OnAnimationComplete( mCurrentAnimName );
				}

			if ( mNextFrame > mEndFrame )
				if ( mIsLooping )
					mNextFrame = mStartFrame;
				else
					mNextFrame = mEndFrame;
		}
	}
}
//---------------------------------------
void MD2Animation::Draw( Window* window )
{
	window->BeginDraw( mInterpTime );
	mModel->DrawInterpolated( mCurrentFrame, mNextFrame, mInterpTime, mTextureIndex );
}
//---------------------------------------
void MD2Animation::PlayAnim( const std::string& name, bool loop )
{
	PlayAnim( name.c_str(), loop );
}
//---------------------------------------
void MD2Animation::PlayAnim( const char* name, bool loop )
{
	const MD2Model::MD2AnimInfo* animInfo = mModel->GetAnimationInfo( name );
	if ( animInfo )
	{
		mCurrentAnimName = animInfo->name;
		mStartFrame = animInfo->start;
		mEndFrame = animInfo->end;
		mCurrentFrame = mStartFrame;
		mNextFrame = mCurrentFrame + 1;
		if ( mNextFrame > mEndFrame )
			mNextFrame = mStartFrame;
		mIsPlaying = true;
		mIsLooping = loop;
	}
}
//---------------------------------------
void MD2Animation::Pause()
{
	mIsPlaying = false;
}
//---------------------------------------
void MD2Animation::Resume()
{
	mIsPlaying = false;
}
//---------------------------------------
bool MD2Animation::IsAnimationPlaying( const char* animName ) const
{
	return mIsPlaying && !strcmp( animName, mCurrentAnimName );
}
//---------------------------------------