#pragma once

#include "XApi.h"
#include <vector>
#include <string>
#include <stdlib.h>

#ifdef WIN32
#include <Windows.h>
#endif

namespace XGAME
{
	XGAME_API char*	outputString( const char* pszFormat, ... );
	
#ifdef WIN32
	/**
	 * 将ANSI转换为UNICODE
	 */
	 XGAME_API int	ansiToUnicode( const char* pSour, wchar_t* pDest, int iWCharLen );

	/**
	 * 将ANSI转换为UTF8
	 */
	 XGAME_API int	ansiToUTF8( const char* pSour, char* pDest, int iCharLen );

	/**
	 * UNICODE转换为ANSI
	 */
	XGAME_API int	unicodeToAnsi( const wchar_t* pSour, char* pDest, int iCharLen );
		
	/**
	 * UNICODE转换为UTF8
	 */
	 XGAME_API int	unicodeToUTF8( wchar_t* pSour, char* pDest, int iCharLen );
	 
	/**
	 * 将UTF8转换为UNICODE
	 */
	 XGAME_API int	utf8ToUnicode( const char* pSour, wchar_t* pDest, int iWCharLen );

	 /**
	  * UTF8转换ANSI
	  */
	 XGAME_API int	utf8ToAnsi( const char* pSour, char* pDest, int iCharLen );
#endif
	
	namespace XGAMESTRING
	{
		XGAME_API bool	split( std::string s, std::vector<std::string>& v, const char* pDlim );
		XGAME_API bool	split( const char* p, std::vector<std::string>& v, const char* pDlim );

		XGAME_API const bool          parseBool( const char* pStr );
		XGAME_API const bool          parseBool( std::string& s );
		
		XGAME_API const char			parseChar( const char* pStr );
		XGAME_API const char			parseChar( std::string& s );
		
		XGAME_API const unsigned char	parseByte( const char* pStr );
		XGAME_API const unsigned char	parseByte( std::string& s );
		
		XGAME_API const short			parseShort( const char* pStr );
		XGAME_API const short			parseShort( std::string& s );
		
		XGAME_API const unsigned short parseWord( const char* pStr );
		XGAME_API const unsigned short parseWord( std::string& s );
		
		XGAME_API const int			parseInt( const char* pStr );
		XGAME_API const int			parseInt( std::string& s );
		
		XGAME_API const unsigned int	parseUInt( const char* pStr );
		XGAME_API const unsigned int	parseUInt( std::string& s );
		
		XGAME_API const unsigned int	parseLong( const char* pStr );
		XGAME_API const unsigned int	parseLong( std::string& s );
		
		XGAME_API const unsigned int	parseDWord( const char* pStr );
		XGAME_API const unsigned int	parseDWord( std::string& s );
		
		XGAME_API const float			parseFloat( const char* pStr );
		XGAME_API const float			parseFloat( std::string& s );
		
		XGAME_API const double		parseDouble( const char* pStr );
		XGAME_API const double		parseDouble( std::string& s );
	}
}