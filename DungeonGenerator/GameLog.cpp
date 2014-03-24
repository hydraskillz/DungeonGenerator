#include "GameLog.h"
#include "Window.h"

#include <cstdarg>

GameLog GameLog::Instance;
const float GameLog::MESSAGE_LIFE = 3.0f;
const int GameLog::MAX_MESSAGES = 10;

//---------------------------------------
GameLog::Message::Message( const char* msg )
	: messageText( msg )
	, life( MESSAGE_LIFE )
{}
//---------------------------------------


//---------------------------------------
GameLog::GameLog()
{}
//---------------------------------------
GameLog::~GameLog()
{}
//---------------------------------------
void GameLog::PostMessage( const char* msg )
{
	mActiveMessages.push_front( Message( msg ) );
}
//---------------------------------------
void GameLog::PostMessageFmt( const char* fmt, ... )
{
	char textFormatBuffer[ 96 ];

	// Apply text formating
	va_list vargs;
	va_start( vargs, fmt );
	vsprintf_s( textFormatBuffer, fmt, vargs );
	va_end( vargs );

	PostMessage( textFormatBuffer );
}
//---------------------------------------
void GameLog::Update( float dt )
{
	int i = 0;
	for ( auto itr = mActiveMessages.begin(); itr != mActiveMessages.end(); ++itr )
	{
		Message& msg = *itr;
		msg.life -= dt;
		if ( i++ >= MAX_MESSAGES )
			msg.life = 0.0f;
	}

	mActiveMessages.remove_if( []( const Message& msg ) -> bool
	{
		return msg.life <= 0.0f;
	});
}
//---------------------------------------
void GameLog::Draw( Window* window )
{
	float y = 0;
	int i = 0;
	for ( auto itr = mActiveMessages.begin(); itr != mActiveMessages.end(); ++itr )
	{
		if ( i++ == MAX_MESSAGES )
			break;
		Message& msg = *itr;

		Color c = Color::WHITE;

		c.a = msg.life;
		window->DrawDebugText( 205, y, c, msg.messageText.c_str() );
		y += 24.0f;
	}
}
//---------------------------------------
void GameLog::Clear()
{
	mActiveMessages.clear();
}
//---------------------------------------