/**
 *	日志工具
 */
#pragma once

#include "XApi.h"
#include <string>
#include <boost/thread/mutex.hpp>

namespace XGAME
{
	using namespace boost;
	
#define MAX_LOG_BUFFER		4096

	/**
	 * 日志模式
	 */
	enum enLogMode
	{
		//无日志模式，用于临时快速切换
		EN_MODE_NOLOG,
		//文件日志模式
		EN_MODE_FILELOG,
	};
	
	class XGAME_API XLogger
	{
	public:
		static XLogger*	getAsioLog();

	public:
		XLogger();
		~XLogger();

		/**
		 * 设置日志文件名称
		 */
		bool	setLogFile( const char* pFileName );

		/**
		 * 写入字符串
		 */
		void	writeLog( const char* pLog, bool immediately );
		
		/**
		 * 写入指定长度字符串
		 */
		void	writeLog( const char* pLog, size_t size, bool immediately );

		/**
		 * 立即直接写入到日志文件
		 */
		void	writeLogToFile( const char* pLog, size_t size );

		/**
		 * 写入时间头
		 */
		void	writeLogTime( bool immediately );

		/**
		 * 写入文件
		 */
		bool	flush();

		/**
		 * 清除文件
		 */
		void	clear();		
		
	protected:
		bool	isFileOpened();
		bool	openFile();
		void	closeFile();
		void	writeFile();

	protected:
		FILE*			m_pLogFile;
		std::string		m_sFileName;
		char			m_szBuffer[MAX_LOG_BUFFER];
		size_t			m_bufferIndex;
		bool			m_bInit;

		boost::mutex	m_mutex;
	};

	/**
	 * 日志工具
	 */
	class XGAME_API XLogUtil
	{
	public:
		XLogUtil();
		~XLogUtil();

		/**
		 * 设置日志模式
		 */
		void	setMode( enLogMode mode );
		/**
		 * 设置日志文件名
		 *
		 * @param useHourFileName	使用小时文件名
		 */
		void	setLogFileName( const char* pFileName, bool useHourFileName );

		/**
		 * 写入日志，支持可变参数
		 */
		void	writeLog( const char* pszFormat, ... );

		/**
		 * 写入字符串
		 */
		void	writeString( const char* pszFormat );
		
		/**
		 * 写入到日志文件
		 */
		void	flush();		

		/**
		 * 清除文件日志内容
		 */
		void	clear();

		/**
		 * 关闭日志文件
		 */
		void	close();

	protected:
		void	updateLogFileName();

	protected:		
		//原始文件名
		std::string		m_sOrgFileName;
		//日志模式
		int				m_enMode;
		//是否按小时分文件名，不是则使用天数区别
		bool			m_bUseHourFileName;
		
		//日志临时缓存空间
		char			m_szText[MAX_LOG_BUFFER];

		//单日志实例
		XLogger			m_log;
		
		boost::mutex	m_mutex;		
	};
}