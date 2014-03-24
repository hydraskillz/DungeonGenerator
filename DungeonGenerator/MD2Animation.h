/*
 * Author      : Matthew Johnson
 * Date        : 19/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include <string>

class MD2Model;
class Window;

class MD2Animation
{
public:
	class OnAnimationCompleteListener
	{
	public:
		virtual void OnAnimationComplete( const char* animName ) = 0;
	};

	MD2Animation( MD2Model* model, int textureIndex=0 );
	~MD2Animation();

	void Update( float dt );
	void Draw( Window* window );

	void PlayAnim( const std::string& name, bool loop=true );
	void PlayAnim( const char* name, bool loop=true );
	void Pause();
	void Resume();
	void SetPlayRate( int rate ) { mRate = rate; }
	void SetOnCompleteListener( OnAnimationCompleteListener* onCompleteListener ) { mOnCompleteCB = onCompleteListener; }
	bool IsAnimationPlaying( const char* animName ) const;
	void SetTextureIndex( int index ) { mTextureIndex = index; }

private:
	MD2Model* mModel;
	OnAnimationCompleteListener* mOnCompleteCB;
	const char* mCurrentAnimName;
	int mCurrentFrame;
	int mNextFrame;
	int mStartFrame;
	int mEndFrame;
	int mRate;
	float mInterpTime;
	int mTextureIndex;
	bool mIsPlaying;
	bool mIsLooping;
};