/*
 * Author      : Matthew Johnson
 * Date        : 15/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include <list>
#include <string>

class Window;

class GameLog
{
private:
	GameLog();
	~GameLog();
public:
	static GameLog Instance;

	void PostMessage( const char* msg );
	void PostMessageFmt( const char* fmt, ... );

	void Update( float dt );
	void Draw( Window* window );

	void Clear();

private:
	static const float MESSAGE_LIFE;
	static const int MAX_MESSAGES;
	struct Message
	{
		Message( const char* msg );

		std::string messageText;
		float life;
	};

	std::list< Message > mActiveMessages;
};