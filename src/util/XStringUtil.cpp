#include "util/XStringUtil.h"
#include <boost/thread/mutex.hpp>
#include <stdarg.h>

#pragma warning( once:4996 )

namespace XGAME
{
#define MAX_LOG_BUFFER		4096

	char*	outputString( const char* pszFormat, ... )
	{
		static boost::mutex	_mutex;
		static char text[MAX_LOG_BUFFER];
		boost::mutex::scoped_lock lock( _mutex );
		va_list args;
		va_start( args, pszFormat );
		vsnprintf( text, MAX_LOG_BUFFER, pszFormat, args );
		va_end( args );
		return text;
	}

#ifdef WIN32
	int	unicodeToAnsi( const wchar_t* pSour, char* pDest, int	iCharLen )  
	{
		if ( pSour == NULL || pDest == NULL || iCharLen == 0 )
		{
			return 0;
		}
		int len = WideCharToMultiByte( CP_ACP, 0, pSour, -1, NULL, 0, NULL, NULL );  
		if ( len == 0 )
		{  
			return 0;
		}  
		iCharLen = iCharLen > len ? len : iCharLen;
		WideCharToMultiByte( CP_ACP, 0, pSour, -1, pDest, len, NULL, NULL );
		if ( len < iCharLen )
		{
			pDest[len] = 0;
		}
		return iCharLen;
	}

	int ansiToUnicode( const char* pSour, wchar_t* pDest, int iWCharLen )
	{
		if ( pSour == NULL || pDest == NULL || iWCharLen == 0 )
		{
			return 0;
		}
		int len;
		//可以修改成指定语言GB2312 936. Shift-JIS 932 UTF-8 65001.
		len = MultiByteToWideChar( CP_ACP, 0, pSour, -1, NULL, 0 );
		if ( len == 0 )
		{
			return 0;
		}
		iWCharLen = iWCharLen > len ? len : iWCharLen;
		MultiByteToWideChar( CP_ACP, 0, pSour, -1, pDest, len );
		if ( len < iWCharLen )
		{
			pDest[len] = 0;
		}		
		return len;
	}

	int	ansiToUTF8( const char* pSour, char* pDest, int iCharLen )
	{
		if ( pSour == NULL || pDest == NULL || iCharLen == 0 )
		{
			return 0;
		}
		wchar_t* pTemp = new wchar_t[iCharLen];
		ansiToUnicode( pSour, pTemp, iCharLen );
		int len = unicodeToUTF8( pTemp, pDest, iCharLen );
		if ( len == 0 )
		{
			return 0;
		}
		delete []  pTemp;
		return len;
	}

	int	unicodeToUTF8( wchar_t* pSour, char* pDest, int iCharLen )
	{
		if ( pSour == NULL || pDest == NULL || iCharLen == 0 )
		{
			return 0;
		}
		int len = WideCharToMultiByte( CP_UTF8, 0, pSour, -1, NULL, 0, NULL, NULL );
		if ( len == 0 )
		{
			return 0;
		}		WideCharToMultiByte( CP_UTF8, 0, pSour, -1, pDest, iCharLen, NULL, NULL );
		if ( len < iCharLen )
		{
			pDest[len] = 0;
		}
		return len;
	}

	int utf8ToUnicode( const char* pSour, wchar_t* pDest, int iWCharLen )
	{
		if ( pSour == NULL || pDest == NULL || iWCharLen == 0 )
		{
			return 0;
		}
		int len = MultiByteToWideChar( CP_UTF8, NULL, pSour, -1, NULL, 0 );
		if ( len == 0 )
		{
			return 0;
		}
		MultiByteToWideChar( CP_UTF8, NULL, pSour, -1, pDest, len );
		if ( len < iWCharLen )
		{
			pDest[len] = 0;
		}
		return len;
	}

	int	utf8ToAnsi( const char* pSour, char* pDest, int iCharLen )
	{
		if ( pSour == NULL || pDest == NULL || iCharLen == 0 )
		{
			return 0;
		}
		wchar_t* pTemp = new wchar_t[iCharLen];
		utf8ToUnicode( pSour, pTemp, iCharLen );
		int len = unicodeToAnsi( pTemp, pDest, iCharLen );
		if ( len == 0 )
		{
			return 0;
		}
		delete []  pTemp;
		return len;
	}
	
#endif

	namespace XGAMESTRING
	{
		bool split( std::string s, std::vector<std::string>& v, const char* pDlim )
		{
			return split( s.c_str(), v, pDlim );
		}
		bool split( const char* pStr, std::vector<std::string>& v, const char* pDlim )
		{
			char* p = NULL;
			p = strtok( (char*)pStr, pDlim );
			while( p )
			{
				v.push_back( std::string( p ) );
				p = strtok( NULL, pDlim );				
			}
			return !v.empty();
		}
		const bool          parseBool( const char* pStr ) { return atoi( pStr ) != 0; }
		const bool          parseBool( std::string& s ) { return parseBool( s.c_str() ); }		

		const char			parseChar( const char* pStr ) { return pStr[0]; }
		const char			parseChar( std::string& s ) { return parseChar( s.c_str() ); }

		const unsigned char	parseByte( const char* pStr ) { return atoi( pStr ); }
		const unsigned char	parseByte( std::string& s ) { return parseByte( s.c_str() ); }

		const short			parseShort( const char* pStr ) { return atoi( pStr ); }
		const short			parseShort( std::string& s ) { return parseShort( s.c_str() ); }

		const unsigned short parseWord( const char* pStr ) { return atoi( pStr ); }
		const unsigned short parseWord( std::string& s ) { return parseWord( s.c_str() ); }

		const int			parseInt( const char* pStr ) { return atoi( pStr ); }
		const int			parseInt( std::string& s ) { return parseInt( s.c_str() ); }

		const unsigned int	parseUInt( const char* pStr ) { return atoi( pStr ); }
		const unsigned int	parseUInt( std::string& s ) { return parseUInt( s.c_str() ); }

		const unsigned int	parseLong( const char* pStr ) { return atol( pStr ); }
		const unsigned int	parseLong( std::string& s ) { return parseLong( s.c_str() ); }

		const unsigned int	parseDWord( const char* pStr ) { return atol( pStr ); }
		const unsigned int	parseDWord( std::string& s ) { return parseDWord( s.c_str() ); }

		const float			parseFloat( const char* pStr ) { return (float)atof( pStr ); }
		const float			parseFloat( std::string& s ) { return parseFloat( s.c_str() ); }

		const double		parseDouble( const char* pStr ) { return (double)atol( pStr ); }
		const double		parseDouble( std::string& s ) { return parseLong( s.c_str() ); }
	}
}