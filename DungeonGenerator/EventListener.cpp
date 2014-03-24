#include "EventListener.h"
#include "HashUtil.h"
#include "Logger.h"

#include <algorithm>

std::map< EventTag, std::vector< EventListener::EventData* > > EventListener::mSlots;

//---------------------------------------
EventListener::EventListener()
{}
//---------------------------------------
EventListener::~EventListener()
{
	UnregisterSlots();
}
//---------------------------------------
void EventListener::RegisterSignal( EventSignal type, EventTag tag )
{
	mSignals[ type ].push_back( tag );
}
//---------------------------------------
void EventListener::RegisterSlot( EventTag tag, const std::string& commands )
{
	EventData* data = new EventData;
	data->mListener = this;
	data->mCommands = commands;
	mSlots[ tag ].push_back( data );
}
//---------------------------------------
void EventListener::UnregisterSlots()
{
	if ( mSlots.size() == 0 ) return;
	for ( auto itr = mSlots.begin(); itr != mSlots.end(); ++itr )
	{
		std::vector< EventData* >& data = itr->second;
		data.erase( std::remove_if( data.begin(), data.end(), [&]( const EventData* eventData )
		{
			return eventData->mListener == this;
		}), data.end() );
	}
}
//---------------------------------------
void EventListener::FireSignal( EventSignal signal )
{
	for ( auto itr = mSignals[ signal ].begin(); itr != mSignals[ signal ].end(); ++itr )
	{
		for ( auto jtr = mSlots[ *itr ].begin(); jtr != mSlots[ *itr ].end(); ++jtr )
		{
			const EventData* data = *jtr;
			DebugPrintf( "Fired event: %u\n", *itr );
			data->mListener->EvaluateCommands( data->mCommands );
		}
	}
}
//---------------------------------------
EventTag EventListener::TagFromString( const std::string& stringTag )
{
	return GenerateHash( stringTag.c_str() );
}
//---------------------------------------
EventSignal EventListener::SignalFromString( const std::string& stringSignal )
{
	if ( stringSignal == "onDeath" )
		return SIGNAL_ON_DEATH;
	if ( stringSignal == "onUse" )
		return SIGNAL_ON_USE;
	WarnFail( "Unknown signal: '%s'\n" );
	return SIGNAL_UNKNOWN;
}
//---------------------------------------