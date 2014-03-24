#include "BitmapFont.h"
#include "Texture.h"
#include "Window.h"
#include "XmlReader.h"

#include <glew.h>

int BitmapFont::TabSize = 4;

//---------------------------------------
BitmapFont::BitmapFont( const char* fntFile )
	: mSpaceWidth( 10 )
{
	XmlReader reader( fntFile );
	XmlReader::XmlReaderIterator fntItr = reader.ReadRoot();
	if ( fntItr.IsValid() )
	{
		if ( fntItr.ValidateXMLChildElemnts( "info,common,pages,chars", "kernings" ) )
		{
			XmlReader::XmlReaderIterator info = fntItr.NextChild( "info" );
			XmlReader::XmlReaderIterator common = fntItr.NextChild( "common" );
			XmlReader::XmlReaderIterator pages = fntItr.NextChild( "pages" );
			XmlReader::XmlReaderIterator chars = fntItr.NextChild( "chars" );

			std::string fontDir = std::string( fntFile).substr( 0, std::string( fntFile ).find_last_of( "/" ) + 1 );

			// Load basic font info
			mSize = info.GetAttributeAsInt( "size" );
			mLineHeight = common.GetAttributeAsInt( "lineHeight" );

			// Load the font pages
			for ( XmlReader::XmlReaderIterator pgItr = pages.NextChild( "page" );
				pgItr.IsValid(); pgItr = pgItr.NextSibling() )
			{
				std::string pgFile = fontDir + pgItr.GetAttributeAsString( "file" );
				Texture2D* page = Texture2D::CreateTexture( pgFile.c_str() );
				page->Load();
				// TODO this is assuming the pages are in order in the xml
				// It is safer to map by id, but this is not needed in normal cases
				mPages.push_back( page );
			}

			// Load glyphs
			for ( XmlReader::XmlReaderIterator chItr = chars.NextChild( "char" );
				  chItr.IsValid(); chItr = chItr.NextSibling() )
			{
				Glyph g;
				int id;

				id = chItr.GetAttributeAsInt( "id" );
				g.x = chItr.GetAttributeAsInt( "x" );
				g.y = chItr.GetAttributeAsInt( "y" );
				g.w = chItr.GetAttributeAsInt( "width" );
				g.h = chItr.GetAttributeAsInt( "height" );
				g.xoff = chItr.GetAttributeAsInt( "xoffset" );
				g.yoff = chItr.GetAttributeAsInt( "yoffset" );
				g.a = chItr.GetAttributeAsInt( "xadvance" );
				g.page = chItr.GetAttributeAsInt( "page" );

				if ( id == 32 )
					mSpaceWidth = g.a;

				mGlyphs[ id ] = g;
			}
		}
	}
}
//---------------------------------------
BitmapFont::~BitmapFont()
{}
//---------------------------------------
void BitmapFont::RenderText( Window* window, float x, float y, const char* text, const Color& color, float scale, int maxLineLength ) const
{
	int c;
	float fx = 0;
	float fy = 0;

	// Adjust the line length by the inverse scale
	if ( maxLineLength > 0 )
	{
		maxLineLength = (int) ( maxLineLength / scale );
	}

	window->SetOrtho();

	glPushMatrix();
	glTranslatef( x, y, 0 );
	glScalef( scale, scale, 1.0f );

	for ( int i = 0; text[i]; ++i )
	{
		// Newline
		if ( text[i] == '\n' )
		{
			fx = 0;
			fy += mLineHeight;
			continue;
		}

		// Tab
		if ( text[i] == '\t' )
		{
			fx += mSpaceWidth * TabSize;
			continue;
		}

		// Use a space for non-printable char
		c = text[i] < ' ' ? ' ' : text[i];

		auto gi = mGlyphs.find( c );
		if ( gi != mGlyphs.end() )
		{
			const Glyph& g = gi->second;
			Texture2D* tx = mPages[ g.page ];

			// Line wrap
			// TODO this could be better (like actually break words correctly)
			if ( maxLineLength > 0 && c != ' ' && ( fx + g.a ) >= maxLineLength )
			{
				fx = 0;
				fy += mLineHeight;
			}

			window->DrawRect( tx, fx + g.xoff, fy + g.yoff, color, g.x, g.y, g.w, g.h );

			// TODO kerning...
			fx += g.a;
		}
	}

	glPopMatrix();

	window->SetPerspective();
}
//---------------------------------------
float BitmapFont::GetLineWidth( const char* text, float scale )
{
	int c;
	float fx = 0;

	for ( int i = 0; text[i]; ++i )
	{
		// Tab
		if ( text[i] == '\t' )
		{
			fx += mSpaceWidth * TabSize;
			continue;
		}

		// Use a space for non-printable char
		c = text[i] < ' ' ? ' ' : text[i];

		auto gi = mGlyphs.find( c );
		if ( gi != mGlyphs.end() )
		{
			const Glyph& g = gi->second;

			// TODO kerning...
			fx += g.a * scale;
		}
	}

	return fx;
}
//---------------------------------------
float BitmapFont::GetLineHeight( const char* text, float scale, int maxLineLength ) const
{
	int c;
	float fx = 0;
	float fy = (float) mLineHeight;

	// Adjust the line length by the inverse scale
	if ( maxLineLength > 0 )
	{
		maxLineLength = (int) ( maxLineLength / scale );
	}

	for ( int i = 0; text[i]; ++i )
	{
		// Newline
		if ( text[i] == '\n' )
		{
			fx = 0;
			fy += mLineHeight;
			continue;
		}

		// Tab
		if ( text[i] == '\t' )
		{
			fx += mSpaceWidth * TabSize;
			continue;
		}

		// Use a space for non-printable char
		c = text[i] < ' ' ? ' ' : text[i];

		auto gi = mGlyphs.find( c );
		if ( gi != mGlyphs.end() )
		{
			const Glyph& g = gi->second;
			Texture2D* tx = mPages[ g.page ];

			// Line wrap
			// TODO this could be better (like actually break words correctly)
			if ( maxLineLength > 0 && c != ' ' && ( fx + g.a ) >= maxLineLength )
			{
				fx = 0;
				fy += mLineHeight;
			}

			// TODO kerning...
			fx += g.a;
		}
	}

	return fy * scale;
}
//---------------------------------------
