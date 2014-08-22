#pragma once

#include "XApi.h"
#include <string>
#include <boost/thread/mutex.hpp>

namespace XGAME
{
	using namespace boost;
	
#define MAX_LOG_BUFFER		4096
		
	class XGAME_API XLogger
	{
	public:
		static XLogger*	getAsioLog();

	public:
		XLogger();
		~XLogger();

		bool	setLogFile( const char* pFileName );

		void	writeLog( const char* pLog, bool immediately );
		void	writeLog( const char* pLog, size_t size, bool immediately );
		void	writeToFileImmed( const char* pLog, size_t size );
		void	writeTime( bool immediately );

		bool	flush();
		void	clear();		
		
	protected:
		bool	isFileOpend();
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
	
	enum enLogMode
	{
		EN_MODE_NOLOG,
		EN_MODE_FILELOG,
	};

	class XGAME_API XLogUtil
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

		//ԭʼ�ļ���
		std::string		m_sOrgFileName;
		//��־ģʽ
		int				m_mode;
		//�Ƿ�Сʱ���ļ���
		bool			m_bUseHourFileName;

		boost::mutex	m_mutex;
		char			m_szText[MAX_LOG_BUFFER];
	};
}