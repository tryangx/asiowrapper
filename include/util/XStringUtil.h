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
		bool	split( std::string s, std::vector<std::string>& v, const char* pDlim );
		bool	split( const char* p, std::vector<std::string>& v, const char* pDlim );

		const bool          parseBool( const char* pStr );
		const bool          parseBool( std::string& s );
		
		const char			parseChar( const char* pStr );
		const char			parseChar( std::string& s );
		
		const unsigned char	parseByte( const char* pStr );
		const unsigned char	parseByte( std::string& s );
		
		const short			parseShort( const char* pStr );
		const short			parseShort( std::string& s );
		
		const unsigned short parseWord( const char* pStr );
		const unsigned short parseWord( std::string& s );
		
		const int			parseInt( const char* pStr );
		const int			parseInt( std::string& s );
		
		const unsigned int	parseUInt( const char* pStr );
		const unsigned int	parseUInt( std::string& s );
		
		const unsigned int	parseLong( const char* pStr );
		const unsigned int	parseLong( std::string& s );
		
		const unsigned int	parseDWord( const char* pStr );
		const unsigned int	parseDWord( std::string& s );
		
		const float			parseFloat( const char* pStr );
		const float			parseFloat( std::string& s );
		
		const double		parseDouble( const char* pStr );
		const double		parseDouble( std::string& s );
	}
}