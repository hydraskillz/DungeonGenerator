/*
 * Author      : Matthew Johnson
 * Date        : 24/Jan/2014
 * Description :
 *   
 */
 
#pragma once

class Color
{
public:
	Color()
		: a( 1.0f ), r( 0.0f ), g( 0.0f ), b( 0.0f )
	{}
	Color( float r, float g, float b, float a=1.0f )
		: a( a ), r( r ), g( g ), b( b )
	{}

	float r, g, b, a;

	// Color constants
	static const Color RED;
	static const Color GREEN;
	static const Color BLUE;
	static const Color YELLOW;
	static const Color CYAN;
	static const Color ORANGE;
	static const Color PURPLE;
	static const Color PINK;
	static const Color WHITE;
	static const Color GREY;
	static const Color LIGHT_GREY;
	static const Color DARK_GREY;
	static const Color BLACK;
	static const Color TRANSPARENCY;
};