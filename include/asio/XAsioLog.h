/**
 *	ASIO日志工具
 */
#pragma once

#include "XApi.h"
#include "util/XLog.h"

namespace XGAME
{
	class XGAME_API XAsioLog
	{
	public:
		static XAsioLog*	getInstance();

	public:
		XAsioLog();

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
		 * 设置日志回调处理
		 */
		void	setLogHandler( std::function< void( const char* pStr ) > handler );

	private:
		XLogUtil	m_logUtil;

		char		m_szText[MAX_LOG_BUFFER];

		std::function< void( const char* pStr ) >	m_funcHandler;
	};
}