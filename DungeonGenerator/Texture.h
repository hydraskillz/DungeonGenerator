#pragma once

#include <string>
#include <map>

struct SDL_Surface;

class Texture2D
{
public:
	// Constructor for graphics engine to use
	Texture2D( int textureId, int w, int h );
private:
	// Called by CreateTexture()
	Texture2D( const char* filename, bool linearFilter=true );
	Texture2D( SDL_Surface* surf );
	~Texture2D();
public:

	bool Load();
	void Unload();

	void Bind();
	static void Unbind();
private:
	bool LoadFromSurface( SDL_Surface* surf );
public:
	inline int GetWidth() const { return mWidth; }
	inline int GetHeight() const { return mHeight; }
	inline unsigned int GetId() const { return mId; }

	// Creates and registers a new Texture2D
	static Texture2D* CreateTexture( const char* filename, bool linearFilter=true );
	static Texture2D* CreateTexture( const std::string& filename, bool linearFilter=true );
	static Texture2D* CreateTexture( const char* name, SDL_Surface* surf, bool linearFilter=true );

	// Only call this if the graphics context was destroyed and texture ids have been invalidated
	// Calling this when texture ids are valid will cause duplicate textures to be loaded
	static void ReloadAllTextures();

	static void DestroyAllTextures();

	// Use this to unload a single texture
	// You should make sure no one else is using it
	static void DestroyTexture( Texture2D*& tx );
	
private:
	static void RegisterTexture( const char* filename, Texture2D* tx );
	static void UnregisterTexture( const char* filename );

	int mWidth, mHeight;
	bool mIsLoaded;
	bool mLinearFilter;
	bool mHasAlpha;
	std::string mFilename;
	unsigned int mId;

	static std::map< std::string, Texture2D* > mTextureRegistry;
};