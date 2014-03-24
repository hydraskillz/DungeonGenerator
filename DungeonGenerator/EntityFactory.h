/*
 * Author      : Matthew Johnson
 * Date        : 21/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "XmlReader.h"
#include "Logger.h"
#include <map>

// Base for all Entity Xml Templates
// Contains some common data Entities generally use
class EntityTemplateBase
{
public:
	EntityTemplateBase( const XmlReader::XmlReaderIterator& xmlItr )
		: mName( xmlItr.GetAttributeAsString( "name" ) )
		, mScale( xmlItr.GetAttributeAsFloat( "scale", 1.0f ) )
		, mTranslate( _mTranslate )
		, mRotate( _mRotate )
	{
		std::vector< float > _tmp;
		xmlItr.GetAttributeAsCSV( "translate", _tmp, "0,0,0" );	// x,y,z
		for ( int i = 0; i < 3; ++i )
			_mTranslate[i] = _tmp[i];
		_tmp.clear();
		xmlItr.GetAttributeAsCSV( "rotate", _tmp, "0,0,0,0" ); // a,x,y,z
		for ( int i = 0; i < 4; ++i )
			_mRotate[i] = _tmp[i];
	}
	const std::string mName;
	const float mScale;
	const float* const mTranslate;
	const float* const mRotate;
private:
	// This is kinda hacky but const float mTranslate[4] isn't possible
	float _mTranslate[3];
	float _mRotate[4];
};

// Base for all Entity Factories
// EntityT is an Entity or subclass
// EntityTemplateT is EntityTemplateBase or subclass
template< typename EntityT, typename EntityTemplateT >
class EntityFactoryBase
{
public:
	// Called by static Entity< EntityT >::LoadTemplatesFromXml()
	void LoadTemplatesFromXml( const XmlReader::XmlReaderIterator& xmlItr );

	// Called by static Entity< EntityT >::CreateEntity()
	// Null is returned if the Xml template did not exist
	EntityT* CreateEntity( const std::string& name );

protected:
	EntityTemplateT* GetTemplate( const std::string& name );

	// This is where you construct the entity from the template data
	virtual void InternalCreateEntity( EntityT& entity, const EntityTemplateT& entityTmpl ) = 0;

	std::map< std::string, EntityTemplateT* > mEntityTemplates;
};

template< typename EntityT, typename EntityTemplateT >
void EntityFactoryBase< EntityT, EntityTemplateT >::LoadTemplatesFromXml( const XmlReader::XmlReaderIterator& xmlItr )
{
	// Free old data when loading new data
	for ( auto itr = mEntityTemplates.begin(); itr != mEntityTemplates.end(); ++itr )
	{
		delete itr->second;
	}
	mEntityTemplates.clear();

	// The assumption is the Xml data is formatted like so:
	// <Entities>
	//  <Entity name="a"/>
	//  <Entity name="b"/>
	// </Entities>
	// xmlItr is assumed to be Entities making itr Entity
	// Entity tag can be named what ever you want but must have a unique name
	// There may be sub elements to Entity which you can parse in your EntityTemplateT class
	for ( XmlReader::XmlReaderIterator itr = xmlItr.NextChild();
		  itr.IsValid(); itr = itr.NextSibling() )
	{
		EntityTemplateT* entityTmpl = new EntityTemplateT( itr );
		mEntityTemplates[ entityTmpl->mName ] = entityTmpl;
	}
}

template< typename EntityT, typename EntityTemplateT >
EntityTemplateT* EntityFactoryBase< EntityT, EntityTemplateT >::GetTemplate( const std::string& name )
{
	EntityTemplateT* entityTmpl = 0;
	auto itr = mEntityTemplates.find( name );
	if ( itr != mEntityTemplates.end() )
	{
		entityTmpl = itr->second;
	}
	else
		WarnFail( "Failed to get Entity Template '%s'\n", name.c_str() );
	return entityTmpl;
}

template< typename EntityT, typename EntityTemplateT >
EntityT* EntityFactoryBase< EntityT, EntityTemplateT >::CreateEntity( const std::string& name )
{
	EntityTemplateT* entityTmpl = GetTemplate( name );
	EntityT* entity = 0;
	if ( entityTmpl )
	{
		entity = new EntityT();
		// This function is where the magic happens
		InternalCreateEntity( *entity, *entityTmpl );
	}
	return entity;
}