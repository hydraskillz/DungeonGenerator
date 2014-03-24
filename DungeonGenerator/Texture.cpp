#include "Texture.h"
#include "StringUtil.h"
#include "Logger.h"

#include <SDL.h>
#include <SDL_image.h>

#include <glew.h>

std::map< std::string, Texture2D* > Texture2D::mTextureRegistry;

//---------------------------------------
void Texture2D::RegisterTexture( const char* key, Texture2D* tx )
{
	mTextureRegistry[ key ] = tx;
}
//---------------------------------------
void Texture2D::UnregisterTexture( const char* key )
{
	auto i = mTextureRegistry.find( key );
	if ( i != mTextureRegistry.end() )
	{
		mTextureRegistry.erase( i );
	}
}
//---------------------------------------
void Texture2D::ReloadAllTextures()
{
	for ( auto itr = mTextureRegistry.begin(); itr != mTextureRegistry.end(); ++itr )
	{
		Texture2D* tx = itr->second;
		tx->Unload();
		tx->mIsLoaded = false;
		tx->Load();
	}
}
//---------------------------------------
Texture2D* Texture2D::CreateTexture( const char* filename, bool linearFilter )
{
	Texture2D*& tx = mTextureRegistry[ filename ];
	if ( !tx )
	{
		tx = new Texture2D( filename, linearFilter );
	}
	return tx;
}
//---------------------------------------
Texture2D* Texture2D::CreateTexture( const std::string& filename, bool linearFilter )
{
	return CreateTexture( filename.c_str(), linearFilter );
}
//---------------------------------------
Texture2D* Texture2D::CreateTexture( const char* name, SDL_Surface* surf, bool linearFilter )
{
	Texture2D*& tx = mTextureRegistry[ name ];
	if ( !tx )
	{
		tx = new Texture2D( surf );
		tx->mLinearFilter = linearFilter;
		tx->mFilename = name;
	}
	return tx;
}
//---------------------------------------
void Texture2D::DestroyAllTextures()
{
	for ( auto itr = mTextureRegistry.begin(); itr != mTextureRegistry.end(); ++itr )
	{
		Texture2D*& tx = itr->second;
		delete tx;
		tx = 0;
	}
	mTextureRegistry.clear();
}
//---------------------------------------
void Texture2D::DestroyTexture( Texture2D*& tx )
{
	Texture2D*& tx2 = mTextureRegistry[ tx->mFilename ];
	if ( tx2 )
	{
		UnregisterTexture( tx->mFilename.c_str() );
	//	delete tx2;
		tx = 0;
	}
}
//---------------------------------------




//---------------------------------------
Texture2D::Texture2D( int textureId, int w, int h )
	: mIsLoaded( true )
	, mFilename( "" )
	, mId( textureId )
	, mWidth( w )
	, mHeight( h )
	, mLinearFilter( true )
	, mHasAlpha( false )
{}
//---------------------------------------
Texture2D::Texture2D( const char* filename, bool linearFilter )
	: mIsLoaded( false )
	, mFilename( filename )
	, mId( 0 )
	, mWidth( 0 )
	, mHeight( 0 )
	, mLinearFilter( linearFilter )
	, mHasAlpha( false )
{
	RegisterTexture( filename, this );
}
//---------------------------------------
Texture2D::Texture2D( SDL_Surface* surf )
	: mIsLoaded( true )
{
	LoadFromSurface( surf );
}
//---------------------------------------
Texture2D::~Texture2D()
{
	glDeleteTextures( 1, &mId );
	mId = 0;
}
//---------------------------------------
SDL_Surface* GuessExtension( const std::string& filename )
{
	SDL_Surface* surf = 0;
	const std::string supportedExtensions[] = 
	{
		".jpg",
		".png",
		".tga"
	};
	for ( unsigned i = 0; i < 3; ++i )
	{
		surf = IMG_Load( ( filename + supportedExtensions[i] ).c_str() );
		if ( surf )
			return surf;
	}
	return surf;
}
//---------------------------------------
bool Texture2D::Load()
{
	if ( !mIsLoaded )
	{
		SDL_Surface* surf = IMG_Load( mFilename.c_str() );

		// Failed to load - see if we can recover
		if ( !surf )
		{
			// Check the extension
			std::string extension = StringUtil::GetFileExtension( mFilename );
			// Try to guess it if it is missing
			if ( extension.empty() )
			{
				surf = GuessExtension( mFilename );
			}

			if ( !surf )
			{
				WarnInfo( "Bad Texture Path '%s' : Searching for texture...\n", mFilename.c_str() );
				// Try the default data folder
				std::string path = "../data/";
				std::string fileWithoutPath = StringUtil::ExtractFilenameFromPath( mFilename );
				// Try to guess it if it is missing
				if ( extension.empty() )
				{
					surf = GuessExtension( path + fileWithoutPath );
				}
				else
				{
					surf = IMG_Load( ( path + fileWithoutPath ).c_str() );
				}
			}
		}

		if ( LoadFromSurface( surf ) )
		{
			SDL_FreeSurface( surf );

			mIsLoaded = true;
		}
	}
	if ( !mIsLoaded )
	{
		WarnFail( "Failed to load image '%s'\n", mFilename.c_str() );
	}
	return mIsLoaded;
}
//---------------------------------------
void Texture2D::Unload()
{
	glDeleteTextures( 1, &mId );
	mId = 0;
	mIsLoaded = false;
	mWidth = 0;
	mHeight = 0;
}
//---------------------------------------
bool Texture2D::LoadFromSurface( SDL_Surface* surf )
{
	if ( surf )
	{
		GLenum format;
		if ( surf->format->BytesPerPixel == 4 )
		{
			format = GL_RGBA;
			mHasAlpha = true;
		}
		else
		{
			format = GL_RGB;
		}

		mWidth = surf->w;
		mHeight = surf->h;

		GLenum filter = mLinearFilter ? GL_LINEAR : GL_NEAREST;

		glGenTextures( 1, &mId );

		glBindTexture( GL_TEXTURE_2D, mId );
		glTexImage2D( GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, surf->pixels );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glBindTexture( GL_TEXTURE_2D, 0 );

//		mTextures.push_back( mId );

		return true;
	}
	return false;
}
//---------------------------------------
void Texture2D::Bind()
{
	if ( !mIsLoaded )
		Load();

	// Enable transparency
	if ( mHasAlpha )
	{
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	else
	{
		glDisable( GL_BLEND );
	}

	glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, mId );
}
//---------------------------------------
void Texture2D::Unbind()
{
	glDisable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
//---------------------------------------