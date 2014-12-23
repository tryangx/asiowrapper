#include "asio/XAsioLog.h"
#include <stdarg.h>

namespace XGAME
{
	XAsioLog* XAsioLog::getInstance()
	{
		static XAsioLog	g_asioLog;
		return &g_asioLog;
	}

	XAsioLog::XAsioLog() : m_funcHandler( nullptr )
	{
	}

	void XAsioLog::setLogFileName( const char* pFileName, bool useHourFileName )
	{
		m_logUtil.setLogFileName( pFileName, useHourFileName );
	}

	void XAsioLog::setLogHandler( std::function< void( const char* pStr ) > handler )
	{
		m_funcHandler = handler;
	}

	void XAsioLog::writeLog( const char* pszFormat, ... )
	{
		va_list args;
		va_start( args, pszFormat );
		vsnprintf( m_szText, MAX_LOG_BUFFER, pszFormat, args );
		va_end( args );

		if ( m_funcHandler )
		{
			m_funcHandler( m_szText );
		}
		m_logUtil.writeLog( m_szText );
	}
}