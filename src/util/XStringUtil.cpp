#include "../../include/util/XStringUtil.h"
#include <boost/thread/mutex.hpp>
#include <stdarg.h>

namespace XGAME
{
#define MAX_LOG_BUFFER		4096

	char* outputString( const char* pszFormat, ... )
	{
		static boost::mutex	_mutex;
		static char text[MAX_LOG_BUFFER];
		boost::mutex::scoped_lock lock( _mutex );
		va_list args;
		va_start( args, pszFormat );
		vsnprintf_s( text, MAX_LOG_BUFFER, pszFormat, args );
		va_end( args );
		return text;
	}
}