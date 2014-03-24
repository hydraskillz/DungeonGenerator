/*
 * Author      : Matthew Johnson
 * Date        : 19/May/2013
 * Description :
 *   
 */
 
#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

// Helper to make strings
#define STRINGIFY( S ) #S

class StringUtil
{
public:
	static std::string StringToLower( const std::string& str );
	static void StringToLower( std::string& str );
	static std::string StringToUpper( const std::string& str );
	static void StringToUpper( std::string& str );
	static void Tokenize( const std::string& str, std::vector< std::string >& tokens, const std::string& pattern=" " );
	static int StringiCmp( const std::string& A, const std::string& B );
	static std::string ExtractFilenameFromPath( const std::string& path, bool keepExtension=true );
	static std::string StripFilenameFromPath( const std::string& path );
	static std::string GetFileExtension( const std::string& filename );
	static std::string SimplifyPath( const std::string& path );
	static std::string LStrip( const std::string& str, const std::string pattern=" " );
	static std::string RStrip( const std::string& str, const std::string pattern=" " );
	static std::string Strip( const std::string& str, const std::string pattern=" " );
	static bool IsNumeric( const char c );

	static bool HexStringToUnsigned( const std::string& str, unsigned& dst )
	{
		std::stringstream ss;
		ss << std::hex << str;
		ss >> dst;
		return !ss.fail();
	}

	template< typename T >
	static bool StringToType( const std::string& str, T* type )
	{
		std::stringstream ss;
		ss << str;
		ss >> *type;
		return !ss.fail();
	}

	// This specialization prevents an issue where stringstream would break
	// a string up at the first space in the general template
	template<>
	static bool StringToType( const std::string& str, std::string* type )
	{
		*type = str;
		return true;
	}

	template< typename T >
	static std::string ToString( const T& type )
	{
		std::stringstream ss;
		ss << type;
		return ss.str();
	}

	template< typename T >
	static std::string VectorToCSVString( const std::vector< T >& v )
	{
		std::string csv;
		if ( v.size() )
			csv = ToString( v.front() );
		for ( typename std::vector< T >::const_iterator itr = v.begin() + 1; itr != v.end(); ++itr )
		{
			csv += "," + ToString( *itr );
		}
		return csv;
	}
};

inline const char* BoolToCString( bool b ) { return b ? "True" : "False"; }
inline std::string BoolToString( bool b ) { return std::string( BoolToCString( b ) ); }