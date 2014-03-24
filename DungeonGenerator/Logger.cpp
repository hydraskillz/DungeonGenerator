#include "Logger.h"

#include <stdio.h>

#ifdef WIN32
#   include <Windows.h>
#   define _WINDOWS_
#endif

#ifdef ANDROID
#   include <android/log.h>

#   define LOG_TAG "MageCore"
#   define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#   define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#   define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#   define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#endif

static uint16 DEFAULT_STYLE = C_FG_WHITE | C_BG_BLACK;
static uint32 gWarnLevel = CONSOLE_WARNING_LEVEL2;
static Console* gDefualtConsole = new SystemConsole();
static Console* gConsole = gDefualtConsole;
#ifdef _WINDOWS_
	static HANDLE ghConsole = GetStdHandle( STD_OUTPUT_HANDLE );
#endif



//---------------------------------------
// Default console is a system console
//---------------------------------------
SystemConsole::SystemConsole()
	: mDefaultStyle( DEFAULT_STYLE )
{}
//---------------------------------------
SystemConsole::~SystemConsole()
{}
//---------------------------------------
void SystemConsole::SetDefaultStyle( uint16 style )
{
	mDefaultStyle = style;
	SetStyle( style );
}
//---------------------------------------
void SystemConsole::SetStyle( uint16 style )
{
#ifdef _WINDOWS_
	SetConsoleTextAttribute( ghConsole, style );
#endif
}
//---------------------------------------
void SystemConsole::Printf( uint16 style, const char* fmt, va_list vargs )
{
#ifdef ANDROID
	char msg[1024];
	vsprintf( msg, fmt, vargs );

	if ( style == CONSOLE_WARNING )
		LOGW( "%s", msg );
	else if ( style == CONSOLE_ERROR )
		LOGE( "%s", msg );
	else if ( style == CONSOLE_INFO )
		LOGI( "%s", msg );
	else
		LOGD( "%s", msg );
#else
	SetStyle( style == CONSOLE_DEFAULT ? mDefaultStyle : style );
	vprintf( fmt, vargs );
	SetStyle( mDefaultStyle );
#endif
}
//---------------------------------------
void SystemConsole::Printf( const char* fmt, va_list vargs )
{
#ifdef ANDROID
	char msg[1024];
	vsprintf( msg, fmt, vargs );
	LOGD( "%s", msg );
#else
	vprintf( fmt, vargs );
#endif
}
//---------------------------------------
bool SystemConsole::OnInput( int code, int mod )
{
	return false;
}
//---------------------------------------
void SystemConsole::Clear()
{
	system( "cls" );
}
//---------------------------------------
void SystemConsole::OnDraw() const
{}
//---------------------------------------
void SystemConsole::OnUpdate( float dt )
{}
//---------------------------------------




//---------------------------------------
void FatalError( const char* fmt, ... )
{
	va_list vargs;
	va_start( vargs, fmt );
	gConsole->Printf( CONSOLE_ERROR, fmt, vargs );
	va_end( vargs );
}
//---------------------------------------
void SetWarningLevel( uint32 level )
{
	gWarnLevel = level;
}
//---------------------------------------
void Warn( uint32 level, const char* fmt, ... )
{
	if ( level & gWarnLevel )
	{
		va_list vargs;
		va_start( vargs, fmt );
		gConsole->Printf( CONSOLE_WARNING, fmt, vargs );
		va_end( vargs );
	}
}
//---------------------------------------
void ConsolePrintf( uint16 style, const char* fmt, ... )
{
	va_list vargs;
	va_start( vargs, fmt );
	gConsole->Printf( style, fmt, vargs );
	va_end( vargs );
}
//---------------------------------------
void ConsolePrintf( const char* fmt, ... )
{
	va_list vargs;
	va_start( vargs, fmt );
	gConsole->Printf( fmt, vargs );
	va_end( vargs );
}
//---------------------------------------
void ConsoleClear()
{
	gConsole->Clear();
}
//---------------------------------------
void ConsoleRender()
{
	gConsole->OnDraw();
}
//---------------------------------------
void ConsoleUpdate( float dt )
{
	gConsole->OnUpdate( dt );
}
//---------------------------------------
bool ConsoleInput( int code, int mod )
{
	return gConsole->OnInput( code, mod );
}
//---------------------------------------
Console* GetGameConsole()
{
	return gConsole;
}
//---------------------------------------
void SetGameConsole( Console* console )
{
	if ( !console )
	{
		gConsole = gDefualtConsole;
	}
	else
	{
		gConsole = console;
	}
}
//---------------------------------------
void SetConsoleDefaultStyle( uint16 style )
{
	gConsole->SetDefaultStyle( style );
}
//---------------------------------------
void SetConsoleStyle( uint16 style )
{
	gConsole->SetStyle( style );
}
//---------------------------------------