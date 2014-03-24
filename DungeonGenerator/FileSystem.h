/*
 * Author      : Matthew Johnson
 * Date        : 4/Jun/2013
 * Description :
 *   Common file system functions.
 */
 
#pragma once

#ifdef ANDROID
#   include <android/asset_manager.h>
#endif

#include <vector>
#include <stdio.h>

static const int FILESYSTEM_MAX_PATH = 256;

enum FileSystemError
{
	FSE_NO_ERROR,
	FSE_FILE_NOT_FOUND,
	FSE_CORRUPT_FILE,
};

void SetPathToData( const char* dataPath, int archiveIndex );
void AddPathToData( const char* dataPath );

bool FileExists( const char* filename );
int GetAllFilesByExtension( const char* directoryPath, const char* ext,
	std::vector< std::string >& filenames );

void ListArchive( const char* archiveName );

// fname is file name relative to root... example:
// data/
//   GameInfo.xml
//   sprites/
//     Player.entity
//
// OpenDataFile( sprites/Player.entity, ... )
// will open the Player.entity file.
int OpenDataFile( const char* fname, char*& _out_file, unsigned int& _out_len );
int WriteDataFile( const char* fname, const char* buffer, unsigned int len );

// Similar to OpenDataFile, but just opens the file and returns the handle
int OpenFile( const char* fname, FILE** hFile );

#ifdef ANDROID
    void InitializeAssetManager( AAssetManager* assetManager );
#endif