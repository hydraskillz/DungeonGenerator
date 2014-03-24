/*
 * Author      : Matthew Johnson
 * Date        : 22/Nov/2013
 * Description :
 *   
 */
 
#pragma once

#include "Color.h"
#include <map>
#include <vector>

class Texture2D;
class Window;

class BitmapFont
{
public:
	// Number of spaces to use for tabs. Default is 4
	static int TabSize;

	BitmapFont( const char* fntFile );
	~BitmapFont();

	void RenderText( Window* window, float x, float y, const char* text, const Color& color=Color::WHITE, float scale=1.0f, int maxLineLength=-1 ) const;

	inline const float GetLineHeight( float scale=1.0f ) const { return mLineHeight * scale; }
	float GetLineHeight( const char* text, float scale=1.0f, int maxLineLength=-1 ) const;
	float GetLineWidth( const char* text, float scale=1.0f );

private:
	struct Glyph
	{
		int x, y;
		int w, h;
		int xoff, yoff;
		int a;
		int page;
	};

	int mSize;
	int mLineHeight;
	int mSpaceWidth;
	std::map< int, Glyph > mGlyphs;
	std::vector< Texture2D* > mPages;
};