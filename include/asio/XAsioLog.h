/**
 *	ASIO��־����
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
		 * ������־�ļ���
		 *
		 * @param useHourFileName	ʹ��Сʱ�ļ���
		 */
		void	setLogFileName( const char* pFileName, bool useHourFileName );

		/**
		 * д����־��֧�ֿɱ����
		 */
		void	writeLog( const char* pszFormat, ... );
		
		/**
		 * ������־�ص�����
		 */
		void	setLogHandler( std::function< void( const char* pStr ) > handler );

	private:
		XLogUtil	m_logUtil;

		char		m_szText[MAX_LOG_BUFFER];

		std::function< void( const char* pStr ) >	m_funcHandler;
	};
}