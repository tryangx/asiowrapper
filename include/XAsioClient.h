#pragma once

#include "XAsioTCP.h"

namespace XASIO
{
	class XClient
	{
	public:
		static size_t	getSendSize();
		static size_t	getRecvSize();

	public:
		XClient( XAsioService& io );
		~XClient();

		/**
		 * ��ȡ����ӿ�
		 */
		TcpClientPtr	getService();
		
		unsigned int	getId() const;
		void			setId( unsigned int id );

		/**
		 * ���õ�ַ
		 */
		void		setAddress( std::string host = "localhost", int port = DEFAULT_XASIO_PORT );
		
		/**
		 * ����
		 */
		void		connect();
		/**
		 * �Ͽ�����
		 */
		void		disconnect();
		
		/**
		 * ����
		 */
		void		send();
		void		send( std::string content );
		void		send( XAsioBuffer& buff );

		void		testSend();		
		void		sendThread();

	public:
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }		

	protected:
		void		init();
		
		/**
		 * ��������
		 * ���ش˺����ɿ�����Ҫ��ȡ�ķ�ʽ����ͷ�ȵ�
		 */
		virtual void	recv();
		
		void		onConnect( TcpSessionPtr session );
		void		onRecv( XAsioBuffer& buff );
		void		onRecvComplete();
		void		onSend( size_t bytesTransferred );
		void		onResolve();
		void		onClose( size_t id );
		void		onLogInfo( const char* pInfo );
		void		onLog( std::string& err );

	protected:
		static size_t		m_sizeSend;
		static size_t		m_sizeRecv;

		XAsioService&		m_service;
		TcpClientPtr		m_ptrTCPClient;
		TcpSessionPtr		m_ptrSession;

		unsigned int		m_id;

		int					m_iPort;
		std::string			m_sHost;

		bool				m_bInit;
		bool				m_bIsConnected;
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;

		boost::thread		m_sendThread;

		deadline_timer		m_deadlineTimer;
		
		std::function<void( std::string& )>			m_funcLogHandler;
	};
}