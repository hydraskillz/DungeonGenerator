/*
 * Author      : Matthew Johnson
 * Date        : 15/Apr/2013
 * Description :
 *   
 */
 
#pragma once

#include "RNG.h"

#include <vector>


template< typename T >
class WeightedRandom
{
public:
	WeightedRandom();
	~WeightedRandom();

	void Add( T value, float weight=1.0f );

	T Evaluate() const;
	float GetTotalWeight() const { return mTotalWeight; }

private:
	std::vector< std::pair< float, T > > mValues;
	float mTotalWeight;
};

//---------------------------------------
// Implementation
//---------------------------------------
template< typename T >
WeightedRandom< T >::WeightedRandom()
	: mTotalWeight( 0 )
{}
//---------------------------------------
template< typename T >
WeightedRandom< T >::~WeightedRandom()
{}
//---------------------------------------
template< typename T >
void WeightedRandom< T >::Add( T value, float weight )
{
	mTotalWeight += weight;
	mValues.push_back( std::make_pair( weight, value ) );
}
//---------------------------------------
template< typename T >
T WeightedRandom< T >::Evaluate() const
{
	float w = RNG::RandomInRange< float >( 0, mTotalWeight );
	for ( typename std::vector< std::pair< float, T > >::const_iterator itr = mValues.begin(); itr != mValues.end(); ++itr )
	{
		if ( w < itr->first ) return itr->second;
		w -= itr->first;
	}

	// To avoid compiler warning
	return mValues[0].second;
}
//---------------------------------------