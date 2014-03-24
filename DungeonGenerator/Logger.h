/*
 * Author      : Matthew Johnson
 * Date        : 15/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include <stdarg.h>
#include "Types.h"

//---------------------------------------
// Foreground colors
#define C_FG_BLACK          0x00
#define C_FG_BLUE           0x01
#define C_FG_GREEN          0x02
#define C_FG_AQUA           0x03
#define C_FG_RED            0x04
#define C_FG_PURPLE         0x05
#define C_FG_YELLOW         0x06
#define C_FG_WHITE          0x07
#define C_FG_GRAY           0x08
#define C_FG_GREY           0x08
#define C_FG_LIGHT_BLUE     0x09
#define C_FG_LIGHT_GREEN    0x0A
#define C_FG_LIGHT_AQUA     0x0B
#define C_FG_LIGHT_RED      0x0C
#define C_FG_LIGHT_PURPLE   0x0D
#define C_FG_LIGHT_YELLOW   0x0E
#define C_FG_LIGHT_WHITE    0x0F
//---------------------------------------
// Background colors
#define C_BG_BLACK          0x00
#define C_BG_BLUE           0x10
#define C_BG_GREEN          0x20
#define C_BG_AQUA           0x30
#define C_BG_RED            0x40
#define C_BG_PURPLE         0x50
#define C_BG_YELLOW         0x60
#define C_BG_WHITE          0x70
#define C_BG_GRAY           0x80
#define C_BG_GREY           0x80
#define C_BG_LIGHT_BLUE     0x90
#define C_BG_LIGHT_GREEN    0xA0
#define C_BG_LIGHT_AQUA     0xB0
#define C_BG_LIGHT_RED      0xC0
#define C_BG_LIGHT_PURPLE   0xD0
#define C_BG_LIGHT_YELLOW   0xE0
#define C_BG_LIGHT_WHITE    0xF0
//---------------------------------------
// Predefined styles
#define CONSOLE_WARNING C_FG_LIGHT_YELLOW | C_BG_BLACK
#define CONSOLE_ERROR   C_FG_LIGHT_RED | C_BG_BLACK
#define CONSOLE_SUCCESS C_FG_LIGHT_GREEN | C_BG_BLACK
#define CONSOLE_INFO    C_FG_LIGHT_WHITE | C_BG_BLACK
#define CONSOLE_DEFAULT -1
//---------------------------------------
// Warnings
#define CONSOLE_WARNING_LEVEL0 0	/* No warnings */
#define CONSOLE_WARNING_LEVEL1 1	/* Critical warnings only */
#define CONSOLE_WARNING_LEVEL2 3	/* Critical + failure warnings */
#define CONSOLE_WARNING_LEVEL3 7	/* All warnings */

#define WarnCrit( ... ) Warn( CONSOLE_WARNING_LEVEL1, __VA_ARGS__ )
#define WarnFail( ... ) Warn( CONSOLE_WARNING_LEVEL2, __VA_ARGS__ )
#define WarnInfo( ... ) Warn( CONSOLE_WARNING_LEVEL3, __VA_ARGS__ )
//---------------------------------------
// Errors
// Macro causes conflicts with external libs
//#define FatalError( ... ) ConsolePrintf( CONSOLE_ERROR, __VA_ARGS__ )
void FatalError( const char* msg, ... );
//---------------------------------------
// Debug
#ifndef MAGE_RELEASE
#	include <assert.h>
#	define DebugPrintf( ... ) ConsolePrintf( CONSOLE_INFO, __VA_ARGS__ )
#	define assertion( expr, ... ) { if ( !(expr) ) { FatalError( __VA_ARGS__ ); assert( expr ); } }
#else
#	define DebugPrintf( ... )
#	define assertion( expr, ... )
#endif
//---------------------------------------
// Console
class Console
{
public:
	Console() {}
	virtual ~Console() {}

	virtual void SetDefaultStyle( uint16 style ) = 0;
	virtual void SetStyle( uint16 style ) = 0;
	virtual void Printf( uint16 style, const char* fmt, va_list vargs ) = 0;
	virtual void Printf( const char* fmt, va_list vargs ) = 0;
	virtual bool OnInput( int code, int mod ) = 0;
	virtual void Clear() = 0;
	virtual void OnDraw() const = 0;
	virtual void OnUpdate( float dt ) = 0;
};
//---------------------------------------
// Default system console
class SystemConsole
	: public Console
{
public:
	SystemConsole();
	virtual ~SystemConsole();

	void SetDefaultStyle( uint16 style );
	void SetStyle( uint16 style );
	void Printf( uint16 style, const char* fmt, va_list vargs );
	void Printf( const char* fmt, va_list vargs );
	bool OnInput( int code, int mod );
	void Clear() ;
	void OnDraw() const;
	void OnUpdate( float dt );

protected:
	uint16 mDefaultStyle;
};
//---------------------------------------
// Warning functions
void SetWarningLevel( uint32 level );
void Warn( uint32 level, const char* fmt, ... );
//---------------------------------------
// Console functions
void SetConsoleDefaultStyle( uint16 style );
void SetConsoleStyle( uint16 style );
void ConsolePrintf( uint16 style, const char* fmt, ... );
void ConsolePrintf( const char* fmt, ... );
void ConsoleClear();
void ConsoleRender();
void ConsoleUpdate( float dt );
bool ConsoleInput( int code, int mod );
Console* GetGameConsole();
void SetGameConsole( Console* console );
//---------------------------------------