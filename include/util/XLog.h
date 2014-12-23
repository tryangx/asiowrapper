/**
 *	��־����
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
	 * ��־ģʽ
	 */
	enum enLogMode
	{
		//����־ģʽ��������ʱ�����л�
		EN_MODE_NOLOG,
		//�ļ���־ģʽ
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
		 * ������־�ļ�����
		 */
		bool	setLogFile( const char* pFileName );

		/**
		 * д���ַ���
		 */
		void	writeLog( const char* pLog, bool immediately );
		
		/**
		 * д��ָ�������ַ���
		 */
		void	writeLog( const char* pLog, size_t size, bool immediately );

		/**
		 * ����ֱ��д�뵽��־�ļ�
		 */
		void	writeLogToFile( const char* pLog, size_t size );

		/**
		 * д��ʱ��ͷ
		 */
		void	writeLogTime( bool immediately );

		/**
		 * д���ļ�
		 */
		bool	flush();

		/**
		 * ����ļ�
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
	 * ��־����
	 */
	class XGAME_API XLogUtil
	{
	public:
		XLogUtil();
		~XLogUtil();

		/**
		 * ������־ģʽ
		 */
		void	setMode( enLogMode mode );
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
		 * д���ַ���
		 */
		void	writeString( const char* pszFormat );
		
		/**
		 * д�뵽��־�ļ�
		 */
		void	flush();		

		/**
		 * ����ļ���־����
		 */
		void	clear();

		/**
		 * �ر���־�ļ�
		 */
		void	close();

	protected:
		void	updateLogFileName();

	protected:		
		//ԭʼ�ļ���
		std::string		m_sOrgFileName;
		//��־ģʽ
		int				m_enMode;
		//�Ƿ�Сʱ���ļ�����������ʹ����������
		bool			m_bUseHourFileName;
		
		//��־��ʱ����ռ�
		char			m_szText[MAX_LOG_BUFFER];

		//����־ʵ��
		XLogger			m_log;
		
		boost::mutex	m_mutex;		
	};
}