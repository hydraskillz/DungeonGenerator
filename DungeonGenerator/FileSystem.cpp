#include "FileSystem.h"
#include "Logger.h"

static const int MAX_FILENAME = 256;
static const int MAX_ARCHIVES = 12;

char PathToData[ MAX_ARCHIVES ][ FILESYSTEM_MAX_PATH ] = { "../data" };
int CurrentNumerOfDataArchives = 1;

#ifdef ANDROID
AAssetManager* gAssetManager;

void InitializeAssetManager( AAssetManager* assetManager )
{
    gAssetManager = assetManager;
}
#endif

//---------------------------------------
char* StripPathFromFile( char* filepath )
{
	char* p = filepath;
	while ( *p )
	{
		if ( *p == '/' || *p == '\\' )
			filepath = p + 1;
		++p;
	}
	return filepath;
}
//---------------------------------------
int _OpenFileFileSystem( const char* fname, char*& _out_file, unsigned int& _out_len )
{
#ifdef ANDROID
    assertion( gAssetManager != NULL, "AssetManaget is NULL! Make sure to call InitializeAssetManager( AAssetManager* assetManager )", "" );
    AAsset* asset;
    asset = AAssetManager_open( gAssetManager, fname, AASSET_MODE_UNKNOWN );
    if ( asset == NULL )
    {
        WarnFail( "Failed to load file: '%s'\n", fname );
        return FSE_FILE_NOT_FOUND;
    }
    _out_len = AAsset_getLength(asset);
    _out_file = new char[ _out_len+1 ];
    _out_file[ _out_len ] = 0;
    _out_len = AAsset_read( asset, _out_file, _out_len );
    AAsset_close( asset );
    return FSE_NO_ERROR;
#else
	char dataPath[ FILESYSTEM_MAX_PATH + 1 ];
	char filenameInArchive[ MAX_FILENAME ];
	int err;
	FILE* fpFile=NULL;

	// Check each data path for file
	for ( int i = CurrentNumerOfDataArchives - 1; i >= 0; --i )
	{
		// Try to open the file
		strncpy_s( filenameInArchive, fname, MAX_FILENAME );
		fopen_s( &fpFile, filenameInArchive, "rb" );
		if ( !fpFile )
		{
			// Build archive name
			strncpy_s( dataPath, PathToData[ i ], FILESYSTEM_MAX_PATH - 1 );
			dataPath[ FILESYSTEM_MAX_PATH ] = '\0';
			strcat_s( dataPath, "/" );

			strcpy_s( filenameInArchive, dataPath );
			strcat_s( filenameInArchive, fname );

			// Try to open the file
			fopen_s( &fpFile, filenameInArchive, "rb" );
			if ( !fpFile )
			{
//				ConsolePrintf( CONSOLE_INFO, "File '%s' not found. Searching additional files...\n", filenameInArchive );
				err = FSE_FILE_NOT_FOUND;
				continue;
			}
		}

		// Get file size
		fseek( fpFile, 0, SEEK_END );
		_out_len = ftell( fpFile );
		rewind( fpFile );

		// Allocate memory for data
		_out_file = new char[ _out_len+1 ];
		_out_file[ _out_len ] = 0;

		// Copy file into memory
		err = fread( _out_file, 1, _out_len, fpFile );
		if ( (unsigned int) err != _out_len )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning : File '%s' failed to read.\n", filenameInArchive );
			err = FSE_CORRUPT_FILE;
		}
		else
		{
			err = FSE_NO_ERROR;
		}

		if ( err == FSE_NO_ERROR )
		{
			ConsolePrintf( CONSOLE_SUCCESS, "Loaded file '%s'\n", filenameInArchive );
		}
		else
		{
			WarnFail( "Failed to load file: '%s'\n", filenameInArchive );
		}

		fclose( fpFile );
		return err;
	}
	return err;
#endif
}
//---------------------------------------
int OpenFile( const char* fname, FILE** hFile )
{
	char dataPath[ FILESYSTEM_MAX_PATH + 1 ];
	char filenameInArchive[ MAX_FILENAME ];
	int err=FSE_NO_ERROR;
	FILE* fpFile=NULL;

	// Check each data path for file
	for ( int i = CurrentNumerOfDataArchives - 1; i >= 0; --i )
	{
		// Try to open the file
		strncpy_s( filenameInArchive, fname, MAX_FILENAME );
		fopen_s( &fpFile, filenameInArchive, "rb" );
		if ( !fpFile )
		{
			// Build archive name
			strncpy_s( dataPath, PathToData[ i ], FILESYSTEM_MAX_PATH - 1 );
			dataPath[ FILESYSTEM_MAX_PATH ] = '\0';
			strcat_s( dataPath, "/" );

			strcpy_s( filenameInArchive, dataPath );
			strcat_s( filenameInArchive, fname );

			// Try to open the file
			fopen_s( &fpFile, filenameInArchive, "rb" );
			if ( !fpFile )
			{
				//				ConsolePrintf( CONSOLE_INFO, "File '%s' not found. Searching additional files...\n", filenameInArchive );
				err = FSE_FILE_NOT_FOUND;
				continue;
			}
		}
		*hFile = fpFile;
		break;
	}
	return err;
}
//---------------------------------------


//---------------------------------------
// Data path functions
//---------------------------------------
void SetPathToData( const char* dataPath, int archiveIndex )
{
	strncpy_s( PathToData[ archiveIndex ], dataPath, FILESYSTEM_MAX_PATH );
}
//---------------------------------------
void AddPathToData( const char* dataPath )
{
	strncpy_s( PathToData[ CurrentNumerOfDataArchives++ ], dataPath, FILESYSTEM_MAX_PATH );
}
//---------------------------------------
int OpenDataFile( const char* fname, char*& _out_file, unsigned int& _out_len )
{
	int err = _OpenFileFileSystem( fname, _out_file, _out_len );
	return err;
}
//---------------------------------------
int WriteDataFile( const char* fname, const char* buffer, unsigned int len )
{
	FILE* fp;
	int bytesWrote = 0;

	char filenameInArchive[ MAX_FILENAME ];
	char* filenameWithoutPath=0;
	char dataPath[ FILESYSTEM_MAX_PATH + 1 ];

	// Build path name
	int i = 0;	// write to primary data location
	strncpy_s( dataPath, PathToData[ i ], FILESYSTEM_MAX_PATH - 1 );
	dataPath[ FILESYSTEM_MAX_PATH ] = '\0';
	strcat_s( dataPath, "/" );

	strcpy_s( filenameInArchive, dataPath );
	strcat_s( filenameInArchive, fname );

	fopen_s( &fp, filenameInArchive, "wb" );

	if ( fp )
	{
		bytesWrote = fwrite( buffer, 1, len, fp );
		fclose( fp );
	}

	return bytesWrote;
}
//---------------------------------------