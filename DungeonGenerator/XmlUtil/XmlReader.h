/*
 * Author      : Matthew Johnson
 * Date        : 9/Mar/2013
 * Description :
 *   
 */
 
#pragma once

#include <string>
#include <stack>
#include <vector>

#include "StringUtil.h"

// From tinyxml2.h
namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
	class XMLAttribute;
	class XMLComment;
	class XMLNode;
	class XMLText;
	class XMLDeclaration;
	class XMLUnknown;

	class XMLPrinter;
}

class XmlReader;
	

class XmlReader
{
public:
	class XmlReaderIterator
	{
	public:
		friend class XmlReader;
		XmlReaderIterator();	// Uninitialized
		XmlReaderIterator( tinyxml2::XMLElement* element, XmlReader* reader );
		
		XmlReaderIterator NextSibling( const char* name=0 ) const;
		XmlReaderIterator NextChild( const char* name=0 ) const;
		XmlReaderIterator PreviousSibling( const char* name=0 ) const;
		XmlReaderIterator PreviousChild( const char* name=0 ) const;

		bool IsValid() const;
		bool HasChildren() const;

		const char* ElementName() const;
		std::string ElementNameAsString() const;
		bool ElementNameEquals( const char* name ) const;
		bool ElementNameEquals( const std::string& name ) const;

		// Same as ElementName(), but checks for null
		const char* operator*() const;

		// Get attribute and return defaultValue if attribute doesn't exist
		// A warning is thrown if the attribute field is not formated as expected
		const char* GetAttributeAsCString    ( const char* name, const char* defaultValue ) const;
		std::string GetAttributeAsString     ( const char* name, const std::string& defaultValue ) const;
		int         GetAttributeAsInt        ( const char* name, const int& defaultValue ) const;
		unsigned    GetAttributeAsUInt       ( const char* name, const unsigned& defaultValue ) const;
		float       GetAttributeAsFloat      ( const char* name, const float& defaultValue ) const;
		bool        GetAttributeAsBool       ( const char* name, const bool& defaultValue ) const;

		// Get attribute and throw assertion and sets Fail() to true if attribute doesn't exist
		// Return value is undefined if assertion is thrown
		const char* GetAttributeAsCString    ( const char* name ) const;
		std::string GetAttributeAsString     ( const char* name ) const;
		int         GetAttributeAsInt        ( const char* name ) const;
		unsigned        GetAttributeAsUInt       ( const char* name ) const;
		float       GetAttributeAsFloat      ( const char* name ) const;
		bool        GetAttributeAsBool       ( const char* name ) const;

		template< typename T >
		void		GetAttributeAsCSV( const char* name, std::vector< T >& out_values ) const;
		template< typename T >
		void		GetAttributeAsCSV( const char* name, std::vector< T >& out_values, const std::string& defaultValue ) const;

		const char* GetPCDataAsCString() const;
		std::string GetPCDataAsString () const;

	// Throws an error and returns false on missing required items
	// and throws a warning and returns true if non optional or required items exist
	// true otherwise
	bool ValidateXMLAttributes( const char* required, const char* optional ) const;
	bool ValidateXMLChildElemnts( const char* required, const char* optional ) const;

	// Query if an attribute is present
	bool HasAttribute( const char* name, bool caseSensitive=true ) const;

	private:
		// Util to convert a n-dim vector encoded as "x,y,...n" to vectorN
		template< typename TVector >
		void ConvertToVector( TVector& vector, unsigned size, const char* attribName,
			const char* attribValue, const char* errorTypename ) const;

		tinyxml2::XMLElement* mCurrentElement;
		XmlReader* mReader;
	};

	XmlReader( const char* filename );
	~XmlReader();

	// Use Read to parse the entire doc in depth order
	// while ( reader.Read() )
	// {
	//     if ( strcmp( reader.ElementName() )
	//     {
	//         // Get element attribs/PCData...
	//     }
	// }
	bool Read();

	// Name of current element
	const char* ElementName() const;
	std::string ElementNameAsString() const;
	bool ElementNameEquals( const char* name ) const;
	bool ElementNameEquals( const std::string& name ) const;
	int GetElementDepth() const;

	// Get an iterator to the root of the doc
	XmlReaderIterator ReadRoot();

	// Returns true if the doc was loaded successfully
	bool IsValid() const;
		
	// Throws an error and returns false on missing required items
	// and throws a warning and returns true if non optional or required items exist
	// true otherwise
	bool ValidateXMLAttributes( const XmlReaderIterator& xmlItr, const char* required, const char* optional );
	bool ValidateXMLChildElemnts( const XmlReaderIterator& xmlItr, const char* required, const char* optional );

	// Query if an attribute is present
	bool HasAttribute( const char* name, bool caseSensitive=true ) const;

	// Get attribute and return defaultValue if attribute doesn't exist
	// A warning is thrown if the attribute field is not formated as expected
	const char* GetAttributeAsCString( const char* name, const char* defaultValue ) const;
	std::string GetAttributeAsString ( const char* name, const std::string& defaultValue ) const;
	int         GetAttributeAsInt    ( const char* name, const int& defaultValue ) const;
	unsigned    GetAttributeAsUInt   ( const char* name, const unsigned& defaultValue ) const;
	float       GetAttributeAsFloat  ( const char* name, const float& defaultValue ) const;
	bool        GetAttributeAsBool   ( const char* name, const bool& defaultValue ) const;

	// Get attribute and throw assertion and sets Fail() to true if attribute doesn't exist
	// Return value is undefined if assertion is thrown
	const char* GetAttributeAsCString( const char* name ) const;
	std::string GetAttributeAsString ( const char* name ) const;
	int         GetAttributeAsInt    ( const char* name ) const;
	unsigned    GetAttributeAsUInt   ( const char* name ) const;
	float       GetAttributeAsFloat  ( const char* name ) const;
	bool        GetAttributeAsBool   ( const char* name ) const;

	template< typename T >
	void		GetAttributeAsCSV( const char* name, std::vector< T >& out_values ) const;
	template< typename T >
	void		GetAttributeAsCSV( const char* name, std::vector< T >& out_values, const std::string& defaultValue ) const;

	const char* GetPCDataAsCString() const;
	std::string GetPCDataAsString () const;

	// Returns true if an error was throw by the last Get*() call
	bool Fail() const { return mError; }

private:
	static const char* DEFAULT_STRING;

	void ClearError() const;
	bool XmlError( int error ) const;
	void XmlAssertError( const XmlReaderIterator& itr, int error, const char* msg, ... ) const;
	void XmlAttribFormatWarning( const XmlReaderIterator& itr, const char* attribName, const char* attribValue, const char* expectedTypeString, const char* msg, ... ) const;

	const char* mFilename;
	mutable bool mError;

	tinyxml2::XMLDocument* mDoc;
	//tinyxml2::XMLElement* mCurrentElement;
	//std::stack< tinyxml2::XMLElement* > mNodeStack;
	XmlReaderIterator mCurrentItr;
	std::stack< XmlReaderIterator > mNodeStack;
};


template< typename T >
void XmlReader::GetAttributeAsCSV( const char* name, std::vector< T >& out_values ) const
{
		mCurrentItr.GetAttributeAsCSV( name, out_values );
}

template< typename T >
void XmlReader::GetAttributeAsCSV( const char* name, std::vector< T >& out_values, const std::string& defaultValue ) const
{
	mCurrentItr.GetAttributeAsCSV( name, out_values, defaultValue );
}

template< typename T >
void XmlReader::XmlReaderIterator::GetAttributeAsCSV( const char* name, std::vector< T >& out_values ) const
{
	mReader->ClearError();
		
	const char* cstr = GetAttributeAsCString( name, 0 );
	
	// Field is missing
	if ( !cstr )
	{
	//	mReader->XmlAssertError( *this, 1,
	//		"No attribute \"%s\" found.\nExpecting CSV \"x,y,...,z\"", name );
		return;
	}

	std::string cvalue = cstr;

	std::vector< std::string > tokens;
	StringUtil::Tokenize( cvalue, tokens, "," );
	for ( std::vector< std::string >::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr )
	{
		T v;
		if ( StringUtil::StringToType( *itr, &v ) )
		{
			out_values.push_back( v );
		}
		else
		{
			mReader->XmlAssertError( *this, 2,
				"Attribute \"%s\" 'Wrong Type'.\nMake sure all values in CSV are of same type.", name );
			return;
		}
	}
}

template< typename T >
void XmlReader::XmlReaderIterator::GetAttributeAsCSV( const char* name, std::vector< T >& out_values, const std::string& defaultValue ) const
{
	mReader->ClearError();

	std::string cvalue = GetAttributeAsString( name, defaultValue );
	std::vector< std::string > tokens;
	StringUtil::Tokenize( cvalue, tokens, "," );
	for ( std::vector< std::string >::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr )
	{
		T v;
		if ( StringUtil::StringToType( *itr, &v ) )
		{
			out_values.push_back( v );
		}
		else
		{
			mReader->XmlAssertError( *this, 2,
				"Attribute \"%s\" 'Wrong Type'.\nMake sure all values in CSV are of same type.", name );
			return;
		}
	}
}