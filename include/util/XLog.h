#pragma once

#include <string>
#include <boost/thread/mutex.hpp>

namespace XASIO
{
	using namespace boost;
	
#define MAX_LOG_BUFFER		4096

	char*	outputString( const char* pszFormat, ... );
	
	class XLogger
	{
	public:
		static XLogger*	getAsioLog();

	public:
		XLogger();
		~XLogger();

		bool	setLogFile( const char* pFileName );

		void	writeLog( const char* pLog, bool immediately );
		void	writeLog( const char* pLog, size_t size, bool immediately );
		void	writeHugeToFile( const char* pLog, size_t size );
		void	writeTime( bool immediately );

		bool	flush();
		void	clear();		
		
	protected:
		bool	openLogFile();
		void	closeLogFile();
		void	writeToFile();

	protected:
		FILE*			m_pLogFile;
		std::string		m_sFileName;
		char			m_szBuffer[MAX_LOG_BUFFER];
		size_t			m_bufferIndex;
		bool			m_bInit;

		boost::mutex	m_mutex;
	};
	
	enum XLogMode
	{
		EN_MODE_NOLOG,
		EN_MODE_FILELOG,
	};

	class XLogUtil
	{
	public:
		XLogUtil();
		~XLogUtil();

		void	setMode( int mode );
		void	setLogFileName( const char* pFileName, bool useHourFileName );

		void	writeLog( const char* pszFormat, ... );
		
		void	flush();		
		void	clear();
		void	close();

	protected:
		void	updateLogFileName();

	protected:
		XLogger			m_log;

		//原始文件名
		std::string		m_sOrgFileName;
		//日志模式
		int				m_mode;
		//是否按小时分文件名
		bool			m_bUseHourFileName;

		boost::mutex	m_mutex;
	};
}