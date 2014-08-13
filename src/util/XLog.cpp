#include "../../include/util/XLog.h"

#include <stdarg.h>
#include <string>

namespace XGAME
{
	//-----------------------------
	/**
	* Êä³ö×Ö·û´®
	*/
	XLogger::XLogger() : m_pLogFile( NULL ), m_bufferIndex( 0 ), m_bInit( false ) {}

	XLogger::~XLogger() { closeLogFile(); }

	bool XLogger::setLogFile( const char* pFileName )
	{
		mutex::scoped_lock lock( m_mutex );
		if ( NULL == pFileName )
		{
			return false;
		}
		if( m_bInit )
		{
			if( m_sFileName == pFileName )
			{
				return true;
			}
			lock.unlock();
			flush();
		}
		m_sFileName = pFileName;
		m_bInit = true;
		return m_bInit;
	}

	bool XLogger::isFileOpend()
	{
		return m_bInit && m_pLogFile;
	}

	bool XLogger::openLogFile()
	{
		mutex::scoped_lock lock( m_mutex );
		if ( m_bInit )
		{
			return 0 == fopen_s( &m_pLogFile, m_sFileName.c_str(), "ab+" );
		}
		return false;
	}

	void XLogger::closeLogFile()
	{
		mutex::scoped_lock lock( m_mutex );
		if( m_pLogFile )
		{
			fclose( m_pLogFile );
			m_pLogFile		= NULL;
			m_bufferIndex	= 0;
		}
	}

	void XLogger::writeLog( const char* pLog, bool immediately )
	{
		size_t size = strlen( pLog );
		writeLog( pLog, size, immediately );
	}
	
	void XLogger::writeLog( const char* pLog, size_t size, bool immediately )
	{
		mutex::scoped_lock lock( m_mutex );
		if( size > MAX_LOG_BUFFER || immediately )
		{
			lock.unlock();
			writeToFileImmed( pLog, size );
		}
		else if( size > 0 && m_bufferIndex + size < MAX_LOG_BUFFER )
		{
			memcpy( &m_szBuffer[m_bufferIndex], pLog, size );
			m_bufferIndex += size;
		}
		else
		{
			lock.unlock();
			if( flush() )
			{
				writeLog( pLog, size, immediately );
			}
		}
	}

	void XLogger::writeToFileImmed( const char* pLog, size_t size )
	{
		if ( isFileOpend() )
		{
			flush();
		}
		if ( openLogFile() )
		{
			fwrite( pLog, 1, size, m_pLogFile );
			closeLogFile();
		}
	}

	bool XLogger::flush()
	{
		if ( openLogFile() )
		{
			writeToFile();
			closeLogFile();
			return m_bufferIndex == 0;
		}
		return false;
	}

	void XLogger::clear()
	{
		mutex::scoped_lock lock( m_mutex );
		if( NULL == m_pLogFile )
		{
			return;
		}
		fclose( m_pLogFile );
		if ( 0 != fopen_s( &m_pLogFile, m_sFileName.c_str(), "wb+" ) )
		{
			m_pLogFile = NULL;
		}
		else
		{
			fclose( m_pLogFile );
		}
	}

	void XLogger::writeToFile()
	{
		mutex::scoped_lock lock( m_mutex );
		if( !m_pLogFile )
		{
			return;
		}
		if( m_bufferIndex > 0 )
		{
			fwrite( m_szBuffer, 1, m_bufferIndex, m_pLogFile );
			m_bufferIndex = 0;
		}
	}

	void XLogger::writeTime( bool immediate )
	{
		time_t tt;
		time( &tt );
		tm gmt;
		localtime_s( &gmt, &tt );
		char output[256];
		sprintf_s( output, 256, "\r\n[%d-%d-%d %d:%d:%d]", gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday, gmt.tm_hour, gmt.tm_min, gmt.tm_sec );
		writeLog( output, immediate );
	}

	//-----------------------------------
	XLogUtil::XLogUtil() : m_sOrgFileName( "srvlog" ), m_mode( EN_MODE_FILELOG ), m_bUseHourFileName( false ) {}
	
	XLogUtil::~XLogUtil() { close(); }

	void XLogUtil::setMode( int mode )
	{
		m_mode = mode;
	}

	void XLogUtil::setLogFileName( const char* pFileName, bool useHourFileName )
	{
		mutex::scoped_lock lock( m_mutex );
		m_bUseHourFileName	= useHourFileName;
		m_sOrgFileName		= pFileName;
	}
	
	void XLogUtil::flush()
	{
		mutex::scoped_lock lock( m_mutex );
		m_log.flush();
	}

	void XLogUtil::clear()
	{
		mutex::scoped_lock lock( m_mutex );
		if ( m_mode == EN_MODE_FILELOG )
		{
			m_log.clear();
		}
	}
	void XLogUtil::close()
	{
		mutex::scoped_lock lock( m_mutex );
		if ( m_mode == EN_MODE_FILELOG )
		{
			m_log.flush();
		}
	}

	void XLogUtil::writeLog( const char* pszFormat, ... )
	{
		mutex::scoped_lock lock( m_mutex );				
		va_list args;
		va_start( args, pszFormat );
		vsnprintf( m_szText, MAX_LOG_BUFFER, pszFormat, args );
		va_end(args);

		updateLogFileName();
		m_log.writeTime( true );
		m_log.writeLog( m_szText, true );
	}

	void XLogUtil::updateLogFileName()
	{
		if ( m_mode == EN_MODE_FILELOG )
		{
			time_t tt;
			time( &tt );
			tm gmt;
			localtime_s( &gmt, &tt );
#define MAX_FILE_PATH	512
			char output[MAX_FILE_PATH];			
			if ( m_bUseHourFileName )
			{
				sprintf_s( output, MAX_FILE_PATH, "%s_%4d-%02d-%02d_%02d.log", m_sOrgFileName.c_str(), gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday, gmt.tm_hour );
			}
			else
			{
				sprintf_s( output, MAX_FILE_PATH, "%s_%4d-%02d-%02d-00.log", m_sOrgFileName.c_str(), gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday );
			}
#undef MAX_FILE_PATH
			m_log.setLogFile( output );
		}
	}
}