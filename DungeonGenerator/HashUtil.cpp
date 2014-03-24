#include "HashUtil.h"

unsigned int GenerateHash( const char* str )
{
	unsigned int hash = 0;
	while ( *str )
	{
		hash &= 0x07ffffff;
		hash *= 31;
		hash += *str;
		++str;
	}
	return hash;
}