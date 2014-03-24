#include "XmlReader.h"
#include "tinyxml2.h"
#include "StringUtil.h"
#include <map>

using namespace tinyxml2;

const char* XmlReader::DEFAULT_STRING = "__DEFAULT__";

enum
{
	XML_ATTRIBUTE_VALIDATION_ERROR = 30,
	XML_ELEMENT_VALIDATION_ERROR
};

//---------------------------------------
// XmlIterator
//---------------------------------------
XmlReader::XmlReaderIterator::XmlReaderIterator()
	: mCurrentElement( nullptr )
	, mReader( nullptr )
{}
//---------------------------------------
XmlReader::XmlReaderIterator::XmlReaderIterator( tinyxml2::XMLElement* element, XmlReader* reader )
	: mCurrentElement( element )
	, mReader( reader )
{}
//---------------------------------------
XmlReader::XmlReaderIterator XmlReader::XmlReaderIterator::NextSibling( const char* name ) const
{
	return XmlReaderIterator( mCurrentElement->NextSiblingElement( name ), mReader );
}
//---------------------------------------
XmlReader::XmlReaderIterator XmlReader::XmlReaderIterator::NextChild( const char* name ) const
{
	return XmlReaderIterator( mCurrentElement->FirstChildElement( name ), mReader );
}
//---------------------------------------
XmlReader::XmlReaderIterator XmlReader::XmlReaderIterator::PreviousSibling( const char* name ) const
{
	return XmlReaderIterator( mCurrentElement->PreviousSiblingElement( name ), mReader );
}
//----------------------------------------
XmlReader::XmlReaderIterator XmlReader::XmlReaderIterator::PreviousChild( const char* name ) const
{
	return XmlReaderIterator( mCurrentElement->LastChildElement( name ), mReader );
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::IsValid() const
{
	return mCurrentElement != nullptr;
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::HasChildren() const
{
	return !mCurrentElement->NoChildren();
}
//---------------------------------------
const char* XmlReader::XmlReaderIterator::operator*() const
{
	return IsValid() ? mCurrentElement->Name() : nullptr;
}
//---------------------------------------
const char* XmlReader::XmlReaderIterator::ElementName() const
{
	return mCurrentElement->Name();
}
//---------------------------------------
std::string XmlReader::XmlReaderIterator::ElementNameAsString() const
{
	return ElementName();
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::ElementNameEquals( const char* name ) const
{
	return !strcmp( name, mCurrentElement->Name() );
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::ElementNameEquals( const std::string& name ) const
{
	return ElementNameEquals( name.c_str() );
}
//---------------------------------------
const char* XmlReader::XmlReaderIterator::GetAttributeAsCString( const char* name, const char* defaultValue ) const
{
	mReader->ClearError();
	const char* result = mCurrentElement->Attribute( name );
	if ( result == nullptr )
		return defaultValue;
	return result;
}
//---------------------------------------
std::string XmlReader::XmlReaderIterator::GetAttributeAsString ( const char* name, const std::string& defaultValue ) const
{
	return GetAttributeAsCString( name, defaultValue.c_str() );
}
//---------------------------------------
int XmlReader::XmlReaderIterator::GetAttributeAsInt( const char* name, const int& defaultValue ) const
{
	mReader->ClearError();
	int value;
	XMLError result;
	result = mCurrentElement->QueryIntAttribute( name, &value );

	if ( result == XML_WRONG_ATTRIBUTE_TYPE )
		mReader->XmlAttribFormatWarning( *this, name, mCurrentElement->Attribute( name ), "Int", "" );

	if ( mReader->XmlError( result ) )
		return defaultValue;

	return value;
}
//---------------------------------------
unsigned XmlReader::XmlReaderIterator::GetAttributeAsUInt( const char* name, const unsigned& defaultValue ) const
{
	mReader->ClearError();
	unsigned value;
	XMLError result;
	result = mCurrentElement->QueryUnsignedAttribute( name, &value );

	if ( result == XML_WRONG_ATTRIBUTE_TYPE )
		mReader->XmlAttribFormatWarning( *this, name, mCurrentElement->Attribute( name ), "Unsigned Int", "" );

	if ( mReader->XmlError( result ) )
		return defaultValue;

	return value;
}
//---------------------------------------
float XmlReader::XmlReaderIterator::GetAttributeAsFloat( const char* name, const float& defaultValue ) const
{
	mReader->ClearError();
	float value;
	XMLError result;
	result = mCurrentElement->QueryFloatAttribute( name, &value );

	if ( result == XML_WRONG_ATTRIBUTE_TYPE )
		mReader->XmlAttribFormatWarning( *this, name, mCurrentElement->Attribute( name ), "Float", "" );

	if ( mReader->XmlError( result ) )
		return defaultValue;
	return value;
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::GetAttributeAsBool( const char* name, const bool& defaultValue ) const
{
	mReader->ClearError();
	bool value;
	XMLError result;
	result = mCurrentElement->QueryBoolAttribute( name, &value );

	if ( result == XML_WRONG_ATTRIBUTE_TYPE )
		mReader->XmlAttribFormatWarning( *this, name, mCurrentElement->Attribute( name ), "Bool", "" );

	if ( mReader->XmlError( result ) )
		return defaultValue;
	return value;
}
//---------------------------------------
const char* XmlReader::XmlReaderIterator::GetAttributeAsCString( const char* name ) const
{
	mReader->ClearError();
	const char* result = mCurrentElement->Attribute( name );
	if ( result == nullptr )
	{
		mReader->XmlAssertError( *this, XML_NO_ATTRIBUTE, "Attribute not found:", name );
		return "";
	}
	return result;
}
//---------------------------------------
std::string XmlReader::XmlReaderIterator::GetAttributeAsString( const char* name ) const
{
	return GetAttributeAsCString( name );
}
//---------------------------------------
int XmlReader::XmlReaderIterator::GetAttributeAsInt( const char* name ) const
{
	mReader->ClearError();
	int value;
	XMLError result;
	result = mCurrentElement->QueryIntAttribute( name, &value );

	mReader->XmlAssertError( *this, result,
		"Expected int\n" );

	if ( mReader->Fail() ) return 0;

	return value;
}
//---------------------------------------
unsigned XmlReader::XmlReaderIterator::GetAttributeAsUInt( const char* name ) const
{
	mReader->ClearError();
	unsigned value;
	XMLError result;
	result = mCurrentElement->QueryUnsignedAttribute( name, &value );

	mReader->XmlAssertError( *this, result,
		"Expected unsigned int.\n" );

	if ( mReader->Fail() ) return 0U;

	return value;
}
//---------------------------------------
float XmlReader::XmlReaderIterator::GetAttributeAsFloat( const char* name ) const
{
	mReader->ClearError();
	float value;
	XMLError result;
	result = mCurrentElement->QueryFloatAttribute( name, &value );

	mReader->XmlAssertError( *this, result,
		"Expected float.\n" );

	if ( mReader->Fail() ) return 0.f;

	return value;
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::GetAttributeAsBool( const char* name ) const
{
	mReader->ClearError();
	bool value;
	XMLError result;
	result = mCurrentElement->QueryBoolAttribute( name, &value );

	mReader->XmlAssertError( *this, result,
		"Expected bool \"true\" or \"false\"\n" );

	if ( mReader->Fail() ) return false;

	return value;
}
//---------------------------------------
const char* XmlReader::XmlReaderIterator::GetPCDataAsCString() const
{
	const char* text = mCurrentElement->GetText();
	if ( text )
	{
		while ( *text == ' ' || *text == '\n' || *text == '\t' ) text++;
		return text;
	}

	printf( "Warning: XmlReader\n File: '%s'\n Element: %s\n No PCData\n",
		mReader->mFilename, mCurrentElement->Name() );

	return "";
}
//---------------------------------------
std::string XmlReader::XmlReaderIterator::GetPCDataAsString () const
{
	return GetPCDataAsCString();
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::ValidateXMLAttributes( const char* required,
	const char* optional ) const
{
	std::vector< std::string > requiredAttribs;
	std::vector< std::string > optionalAttribs;
	std::vector< std::string > missingRequiredAttribs;
	std::vector< std::string > unexpectedAttribs;

	std::map< std::string, int > attributes;

	std::string errorMsg;

	for ( const XMLAttribute* xmlAttrib = mCurrentElement->FirstAttribute();
		  xmlAttrib; xmlAttrib = xmlAttrib->Next() )
	{
		attributes[ xmlAttrib->Name() ];
	}

	StringUtil::Tokenize( required, requiredAttribs, "," );
	StringUtil::Tokenize( optional, optionalAttribs, "," );

	// Check required
	for ( auto itr = requiredAttribs.begin(); itr != requiredAttribs.end(); ++itr )
	{
		auto found = attributes.find( *itr );

		// Atrrib is missing
		if ( found == attributes.end() )
		{
			if ( !itr->empty() )
				missingRequiredAttribs.push_back( *itr );
		}
		else
		{
			// Duplicate required
			if ( found->second == 1 )
			{
				// Not sure if user knows what they are doing
			}
			else
			{
				found->second = 1;
			}
		}
	}

	// Check optional
	for ( auto itr = optionalAttribs.begin(); itr != optionalAttribs.end(); ++itr )
	{
		auto found = attributes.find( *itr );

		// Optional attrib
		if ( found != attributes.end() )
		{
			
			if ( found->second != 0 )
			{
				// Attrib was required and optional??
				if ( found->second == 1 )
				{
					// Not sure if user knows what they are doing
				}
				// Duplicate optional
				else if ( found->second == 2 )
				{
					// Not sure if user knows what they are doing
				}
			}
			else
			{
				found->second = 2;
			}
		}
		// else missing optional - don't care
	}

	// Check unexpected attribs
	for ( auto itr = attributes.begin(); itr != attributes.end(); ++itr )
	{
		// This attrib was not set by either required or optional - report it as unexpected
		if ( itr->second == 0 )
		{
			unexpectedAttribs.push_back( itr->first );
		}
	}

	// Report missing attribs
	for ( auto itr = missingRequiredAttribs.begin(); itr != missingRequiredAttribs.end(); ++itr )
	{
		//printf( "Missing attrib: %s\n", itr->c_str() );
		errorMsg += "Missing required attribute: \"" + *itr + "\"\n";
	}
	
	// Report unexpected attribs
	for ( auto itr = unexpectedAttribs.begin(); itr != unexpectedAttribs.end(); ++itr )
	{
		//printf( "Unexpected Attribute: \"%s\"\n", itr->c_str() );
		errorMsg += "Unexpected Attribute: \"" + *itr + "\"\n";
		// Check for case-miss match possibility
		for ( auto jtr = requiredAttribs.begin(); 
				jtr != requiredAttribs.end(); ++jtr )
		{
			// Check for case miss-match possibility
			if ( !_stricmp( itr->data(), jtr->data() ) )
			{
				//printf( "Did you mean: \"%s\"?\n", jtr->c_str() );
				errorMsg += ">Did you mean: \"" + *jtr + "\"?\n";
			}
		}
		for ( auto jtr = optionalAttribs.begin(); 
				jtr != optionalAttribs.end(); ++jtr )
		{
			// Check for case miss-match possibility
			if ( !_stricmp( itr->data(), jtr->data() ) )
			{
				//printf( "Did you mean: \"%s\"?\n", jtr->c_str() );
				errorMsg += ">Did you mean: \"" + *jtr + "\"?\n";
			}
		}
	}

	if ( errorMsg.length() > 0 )
	{
		mReader->XmlAssertError( *this, XML_ATTRIBUTE_VALIDATION_ERROR, errorMsg.c_str() );
	}
	
	return missingRequiredAttribs.size() == 0 && unexpectedAttribs.size() == 0;
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::ValidateXMLChildElemnts( const char* required,
	const char* optional ) const
{
	std::vector< std::string > requiredElements;
	std::vector< std::string > optionalElements;
	std::vector< std::string > missingRequiredElements;
	std::vector< std::string > unexpectedElements;

	std::map< std::string, int > elements;

	std::string errorMsg;

	for ( const XMLElement* xmlElem = mCurrentElement->FirstChildElement();
		  xmlElem; xmlElem = xmlElem->NextSiblingElement() )
	{
		elements[ xmlElem->Name() ];
	}

	StringUtil::Tokenize( required, requiredElements, "," );
	StringUtil::Tokenize( optional, optionalElements, "," );

	// Check required
	for ( auto itr = requiredElements.begin(); itr != requiredElements.end(); ++itr )
	{
		auto found = elements.find( *itr );

		// Element is missing
		if ( found == elements.end() )
		{
			missingRequiredElements.push_back( *itr );
		}
		else
		{
			// Duplicate required
			if ( found->second == 1 )
			{
				// Not sure if user knows what they are doing
			}
			else
			{
				found->second = 1;
			}
		}
	}

	// Check optional
	for ( auto itr = optionalElements.begin(); itr != optionalElements.end(); ++itr )
	{
		auto found = elements.find( *itr );

		// Optional element
		if ( found != elements.end() )
		{
			
			if ( found->second != 0 )
			{
				// Element was required and optional??
				if ( found->second == 1 )
				{
					// Not sure if user knows what they are doing
				}
				// Duplicate optional
				else if ( found->second == 2 )
				{
					// Not sure if user knows what they are doing
				}
			}
			else
			{
				found->second = 2;
			}
		}
		// else missing optional - don't care
	}

	// Check unexpected elements
	for ( auto itr = elements.begin(); itr != elements.end(); ++itr )
	{
		// This attrib was not set by either required or optional - report it as unexpected
		if ( itr->second == 0 )
		{
			unexpectedElements.push_back( itr->first );
		}
	}

	// Report missing elements
	for ( auto itr = missingRequiredElements.begin(); itr != missingRequiredElements.end(); ++itr )
	{
		if ( !itr->empty() )
		{
			errorMsg += "Missing required element: \"" + *itr + "\"\n";
		}
	}
	
	// Report unexpected elements
	for ( auto itr = unexpectedElements.begin(); itr != unexpectedElements.end(); ++itr )
	{
		//printf( "Unexpected Attribute: \"%s\"\n", itr->c_str() );
		errorMsg += "Unexpected Element: \"" + *itr + "\"\n";
		// Check for case-miss match possibility
		for ( auto jtr = requiredElements.begin(); 
				jtr != requiredElements.end(); ++jtr )
		{
			// Check for case miss-match possibility
			if ( !_stricmp( itr->data(), jtr->data() ) )
			{
				//printf( "Did you mean: \"%s\"?\n", jtr->c_str() );
				errorMsg += ">Did you mean: \"" + *jtr + "\"?\n";
			}
		}
		for ( auto jtr = optionalElements.begin(); 
				jtr != optionalElements.end(); ++jtr )
		{
			// Check for case miss-match possibility
			if ( !_stricmp( itr->data(), jtr->data() ) )
			{
				//printf( "Did you mean: \"%s\"?\n", jtr->c_str() );
				errorMsg += ">Did you mean: \"" + *jtr + "\"?\n";
			}
		}
	}

	if ( errorMsg.length() > 0 )
	{
		mReader->XmlAssertError( *this, XML_ELEMENT_VALIDATION_ERROR, errorMsg.c_str() );
	}
	
	return missingRequiredElements.size() == 0 && unexpectedElements.size() == 0;
}
//---------------------------------------
bool XmlReader::XmlReaderIterator::HasAttribute( const char* name, bool caseSensitive ) const
{
	bool found = false;
	for ( const XMLAttribute* xmlAttrib = mCurrentElement->FirstAttribute();
		xmlAttrib; xmlAttrib = xmlAttrib->Next() )
	{
		if ( !caseSensitive )
			found = !StringUtil::StringiCmp( name, xmlAttrib->Name() );
		else
			found = !strcmp( xmlAttrib->Name(), name );
		if ( found )
			break;
	}
	return found;
}
//---------------------------------------
template< typename TVector >
void XmlReader::XmlReaderIterator::ConvertToVector( TVector& vector, unsigned size, const char* attribName,
	const char* attribValue, const char* errorTypename ) const
{
	std::vector< std::string > tokens;
	StringUtil::Tokenize( attribValue, tokens, "," );

	if ( tokens.size() != size )
	{
		mReader->XmlAttribFormatWarning( *this, attribName, attribValue, errorTypename,
			tokens.size() < size ? "Too few elements." : "Too many elements."  );
	}
	else
	{
		for ( unsigned i = 0; i < size; ++i )
		{
			if ( !StringUtil::StringToType( tokens[i], &vector[i] ) )
			{
				mReader->XmlAttribFormatWarning( *this, attribName, attribValue, errorTypename,
					"Element %d failed to convert to float: \"%s\"",
					i+1, tokens[i].c_str() );
			}
		}
	}
}
//---------------------------------------


//---------------------------------------
// XmlReader
//---------------------------------------
XmlReader::XmlReader( const char* filename )
	: mFilename( filename )
	, mError( false )
{
	mDoc = new tinyxml2::XMLDocument;
	if ( mDoc->LoadFile( filename ) )
	{
		delete mDoc;
		mDoc = 0;
	}
	else
	{
		mCurrentItr = XmlReaderIterator( mDoc->RootElement(), this );
	}
}
//---------------------------------------
XmlReader::~XmlReader()
{
	delete mDoc;
}
//---------------------------------------
bool XmlReader::Read()
{
	XmlReaderIterator nextChild = mCurrentItr.NextChild();
	XmlReaderIterator nextSibling =  mCurrentItr.NextSibling();
	if ( nextChild.IsValid() )
	{
		mNodeStack.push( mCurrentItr );
		mCurrentItr = nextChild;
	}
	else if ( nextSibling.IsValid() )
	{
		mCurrentItr = nextSibling;
	}
	else if ( !mNodeStack.empty() )
	{
		do
		{
			mCurrentItr = mNodeStack.top();
			mNodeStack.pop();
		} while ( !mCurrentItr.NextSibling().IsValid() && !mNodeStack.empty() );

		if ( !mCurrentItr.NextSibling().IsValid() ) return false;

		mCurrentItr = mCurrentItr.NextSibling();
	}
	else
	{
		return false;
	}
	return true;

	/*
	if ( mCurrentElement->FirstChildElement() )
	{
		mNodeStack.push( mCurrentElement );
		mCurrentElement = mCurrentElement->FirstChildElement();
	}
	else if ( mCurrentElement->NextSiblingElement() )
	{
		mCurrentElement = mCurrentElement->NextSiblingElement();
	}
	else if ( !mNodeStack.empty() )
	{
		do
		{
			mCurrentElement = mNodeStack.top();
			mNodeStack.pop();
		} while ( mCurrentElement->NextSiblingElement() == nullptr && !mNodeStack.empty() );

		if ( mCurrentElement->NextSiblingElement() == nullptr ) return false;

		mCurrentElement = mCurrentElement->NextSiblingElement();
	}
	else
	{
		return false;
	}
	return true;
	*/
}
//---------------------------------------
const char* XmlReader::ElementName() const
{
	return mCurrentItr.ElementName();
	//return mCurrentElement->Name();
}
//---------------------------------------
std::string XmlReader::ElementNameAsString() const
{
	return ElementName();
}
//---------------------------------------
bool XmlReader::ElementNameEquals( const char* name ) const
{
	return mCurrentItr.ElementNameEquals( name );
}
//---------------------------------------
bool XmlReader::ElementNameEquals( const std::string& name ) const
{
	return ElementNameEquals( name.c_str() );
}
//---------------------------------------
int XmlReader::GetElementDepth() const
{
	return mNodeStack.size();
}
//---------------------------------------
XmlReader::XmlReaderIterator XmlReader::ReadRoot()
{
	return XmlReaderIterator( mDoc ? mDoc->RootElement() : 0, this );
}
//---------------------------------------
bool XmlReader::IsValid() const
{
	return mDoc != 0;
}
//---------------------------------------
const char* XmlReader::GetAttributeAsCString( const char* name, const char* defaultValue ) const
{
	return mCurrentItr.GetAttributeAsCString( name, defaultValue );
}
//---------------------------------------
std::string XmlReader::GetAttributeAsString ( const char* name, const std::string& defaultValue ) const
{
	return GetAttributeAsCString( name, defaultValue.c_str() );
}
//---------------------------------------
int XmlReader::GetAttributeAsInt( const char* name, const int& defaultValue ) const
{
	return mCurrentItr.GetAttributeAsInt( name, defaultValue );
}
//---------------------------------------
unsigned XmlReader::GetAttributeAsUInt( const char* name, const unsigned& defaultValue ) const
{
	return mCurrentItr.GetAttributeAsUInt( name, defaultValue );
}
//---------------------------------------
float XmlReader::GetAttributeAsFloat( const char* name, const float& defaultValue ) const
{
	return mCurrentItr.GetAttributeAsFloat( name, defaultValue );
}
//---------------------------------------
bool XmlReader::GetAttributeAsBool( const char* name, const bool& defaultValue ) const
{
	return mCurrentItr.GetAttributeAsBool( name, defaultValue );
}
//---------------------------------------
const char* XmlReader::GetAttributeAsCString( const char* name ) const
{
	return mCurrentItr.GetAttributeAsCString( name );
}
//---------------------------------------
std::string XmlReader::GetAttributeAsString( const char* name ) const
{
	return GetAttributeAsCString( name );
}
//---------------------------------------
int XmlReader::GetAttributeAsInt( const char* name ) const
{
	return mCurrentItr.GetAttributeAsInt( name );
}
//---------------------------------------
unsigned XmlReader::GetAttributeAsUInt( const char* name ) const
{
	return mCurrentItr.GetAttributeAsUInt( name );
}
//---------------------------------------
float XmlReader::GetAttributeAsFloat( const char* name ) const
{
	return mCurrentItr.GetAttributeAsFloat( name );
}
//---------------------------------------
bool XmlReader::GetAttributeAsBool( const char* name ) const
{
	return mCurrentItr.GetAttributeAsBool( name );
}
//---------------------------------------
const char* XmlReader::GetPCDataAsCString() const
{
	return mCurrentItr.GetPCDataAsCString();
}
//---------------------------------------
std::string XmlReader::GetPCDataAsString () const
{
	return GetPCDataAsCString();
}
//---------------------------------------
void XmlReader::ClearError() const
{
	mError = false;
}
//---------------------------------------
bool XmlReader::XmlError( int error ) const
{
	return error != 0;
}
//---------------------------------------
void XmlReader::XmlAssertError( const XmlReaderIterator& itr, int error, const char* msg, ... ) const
{
	static std::map< int, const char* > XmlErrorMapping;
	static bool _init = false;
	if ( !_init )
	{
		_init = true;
		XmlErrorMapping[ XML_NO_ERROR                       ] = "XML_NO_ERROR";
		XmlErrorMapping[ XML_SUCCESS                        ] = "XML_SUCCESS";
		XmlErrorMapping[ XML_NO_ATTRIBUTE                   ] = "XML_NO_ATTRIBUTE";
		XmlErrorMapping[ XML_WRONG_ATTRIBUTE_TYPE           ] = "XML_WRONG_ATTRIBUTE_TYPE";
		XmlErrorMapping[ XML_ERROR_FILE_NOT_FOUND           ] = "XML_ERROR_FILE_NOT_FOUND";
		XmlErrorMapping[ XML_ERROR_FILE_COULD_NOT_BE_OPENED ] = "XML_ERROR_FILE_COULD_NOT_BE_OPENED";
		XmlErrorMapping[ XML_ERROR_FILE_READ_ERROR			] = "XML_ERROR_FILE_READ_ERROR";
		XmlErrorMapping[ XML_ERROR_ELEMENT_MISMATCH			] = "XML_ERROR_ELEMENT_MISMATCH";
		XmlErrorMapping[ XML_ERROR_PARSING_ELEMENT			] = "XML_ERROR_PARSING_ELEMENT";
		XmlErrorMapping[ XML_ERROR_PARSING_ATTRIBUTE		] = "XML_ERROR_PARSING_ATTRIBUTE";
		XmlErrorMapping[ XML_ERROR_IDENTIFYING_TAG			] = "XML_ERROR_IDENTIFYING_TAG";
		XmlErrorMapping[ XML_ERROR_PARSING_TEXT				] = "XML_ERROR_PARSING_TEXT";
		XmlErrorMapping[ XML_ERROR_PARSING_CDATA			] = "XML_ERROR_PARSING_CDATA";
		XmlErrorMapping[ XML_ERROR_PARSING_COMMENT			] = "XML_ERROR_PARSING_COMMENT";
		XmlErrorMapping[ XML_ERROR_PARSING_DECLARATION		] = "XML_ERROR_PARSING_DECLARATION";
		XmlErrorMapping[ XML_ERROR_PARSING_UNKNOWN			] = "XML_ERROR_PARSING_UNKNOWN";
		XmlErrorMapping[ XML_ERROR_EMPTY_DOCUMENT			] = "XML_ERROR_EMPTY_DOCUMENT";
		XmlErrorMapping[ XML_ERROR_MISMATCHED_ELEMENT		] = "XML_ERROR_MISMATCHED_ELEMENT";
		XmlErrorMapping[ XML_ERROR_PARSING					] = "XML_ERROR_PARSING";
		XmlErrorMapping[ XML_CAN_NOT_CONVERT_TEXT			] = "XML_CAN_NOT_CONVERT_TEXT";
		XmlErrorMapping[ XML_NO_TEXT_NODE					] = "XML_NO_TEXT_NODE";
		XmlErrorMapping[ XML_ATTRIBUTE_VALIDATION_ERROR     ] = "XML_ATTRIBUTE_VALIDATION_ERROR";
		XmlErrorMapping[ XML_ELEMENT_VALIDATION_ERROR       ] = "XML_ELEMENT_VALIDATION_ERROR";
	}

	const char* elemName = "";
	char attribBuffer[ 1024 ] = { 0 };
	if ( itr.IsValid() )
	{
		//elemName = mCurrentElement->Name();
		elemName = itr.mCurrentElement->Name();
		//const XMLAttribute* xmlAttrib = mCurrentElement->FirstAttribute();
		const XMLAttribute* xmlAttrib = itr.mCurrentElement->FirstAttribute();
		int offset = 0;
		while ( xmlAttrib )
		{
			const char* name = xmlAttrib->Name();
			const char* value = xmlAttrib->Value();

#ifdef ANDROID
            sprintf( attribBuffer + offset, "%s = %s\n", name, value );
#else
			sprintf_s( attribBuffer + offset, 1024 - offset, "%s = %s\n", name, value );
#endif
			offset += strlen( name ) + strlen( value ) + 4;
			xmlAttrib = xmlAttrib->Next();
		}
	}
	attribBuffer[ 1023 ] = 0;

	if ( XmlError( error ) )
	{
		char msgBuffer[1024];
		va_list vargs;
		va_start( vargs, msg );
		vsprintf_s( msgBuffer, msg, vargs );
		va_end( vargs );

		printf(
"\nXML Error\n\n\
File:\n'%s'\n\n\
Element:\n\
%s\n\n\
Attributes:\n\
%s\n\n\
Error:\n\
%s\n\n\
Additional Info:\n\
%s\n",
		mFilename, elemName, attribBuffer, XmlErrorMapping[ error ], msgBuffer );

		mError = true;
	}
}
//---------------------------------------
void XmlReader::XmlAttribFormatWarning( const XmlReaderIterator& itr, const char* attribName,
	const char* attribValue, const char* expectedTypeString, const char* msg, ... ) const
{
	char msgBuffer[1024];
	va_list vargs;
	va_start( vargs, msg );
	vsprintf_s( msgBuffer, msg, vargs );
	va_end( vargs );

	XmlAssertError( itr, XML_WRONG_ATTRIBUTE_TYPE,
		"For attribute \"%s\":\nExpected %s.\nGot \"%s\".\n%s", attribName,
		expectedTypeString, attribValue, msgBuffer );
}
//---------------------------------------