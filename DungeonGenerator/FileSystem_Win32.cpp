#include "FileSystem.h"

#ifdef WIN32

// windows
#include <Windows.h>
#include <strsafe.h>

//---------------------------------------
bool FileExists( const char* filename )
{
	DWORD attrib = GetFileAttributes( filename );
	return ( attrib != INVALID_FILE_ATTRIBUTES &&
		  !( attrib & FILE_ATTRIBUTE_DIRECTORY ) );
}
//---------------------------------------
int GetAllFilesByExtension( const char* directoryPath, const char* ext,
	std::vector< std::string >& filenames )
{
	WIN32_FIND_DATA findData;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	StringCchCopy( szDir, MAX_PATH, directoryPath );
	StringCchCat( szDir, MAX_PATH, ext );

	hFind = FindFirstFile( szDir, &findData );

	if ( hFind == INVALID_HANDLE_VALUE )
	{
		// No files found
		return 0;
	}

	do
	{
		if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{}
		else
		{
			filenames.push_back( findData.cFileName );
		}

	} while ( FindNextFile( hFind, &findData ) != 0 );

	FindClose( hFind );

	return (int) filenames.size();
}
//---------------------------------------
#endif /* WIN32 */