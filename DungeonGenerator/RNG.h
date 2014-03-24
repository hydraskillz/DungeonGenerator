/*
 * Author      : Matthew Johnson
 * Date        : 15/Apr/2013
 * Description :
 *   
 */
 
#pragma once


class RNG
{
public:
	~RNG();

	// [0, 1]
	static float RandomUnit()
	{
		return static_cast< float >( Rand() ) / static_cast< float >( RandMax );
	}

	// [-1, 1]
	static float RandomUniform()
	{
		return static_cast< float >( 2 * RandomUnit() - 1 );
	}

	// [min, max]
	template< typename TReal >
	static TReal RandomInRange( TReal min, TReal max )
	{
		return static_cast< TReal >( min + ( max - min ) * RandomUnit() );
	}

	static unsigned RandomIndex( unsigned size )
	{
		return size == 0 ? 0 : Rand() % size;
	}

	static void SetRandomSeed( unsigned long seed )
	{
		Seed = seed;
	}

	static unsigned long GetSeed()
	{
		return Seed;
	}

	static unsigned int Rand()
	{
		Seed = Seed * 1103515245 + 12345;
		return (unsigned int) ( Seed / 65536 ) % 32768;
	}

	static const unsigned long RandMax = 32767;

protected:
	RNG();

private:
	static unsigned long Seed;
};
