/*
 * Author      : Matthew Johnson
 * Date        : 27/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include <map>
#include <vector>
#include <string>

typedef int EventSignal;
typedef unsigned int EventTag;

class EventListener
{
public:
	enum
	{
		SIGNAL_ON_DEATH,
		SIGNAL_ON_USE,
		SIGNAL_UNKNOWN
	};

	EventListener();
	virtual ~EventListener();

	
	void RegisterSignal( EventSignal type, EventTag tag );
	void RegisterSlot( EventTag tag, const std::string& commands );
	// Unregister all slots this object is registered for
	void UnregisterSlots();
	// Signal all objects listening for tag
	void FireSignal( EventSignal signal );
	// Hash a string into a tag
	static EventTag TagFromString( const std::string& stringTag );
	// Get signal enum from string
	static EventSignal SignalFromString( const std::string& stringSignal );
	// This is called in response to receiving a signal from a slot
	virtual void EvaluateCommands( const std::string& commands ) = 0;

private:
	struct EventData
	{
		EventListener* mListener;
		std::string mCommands;
	};

	std::map< EventSignal, std::vector< EventTag > > mSignals;
	static std::map< EventTag, std::vector< EventData* > > mSlots;
};