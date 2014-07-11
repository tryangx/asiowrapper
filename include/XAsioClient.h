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
		 * 获取服务接口
		 */
		TcpClientPtr	getService();
		
		unsigned int	getId() const;
		void			setId( unsigned int id );

		/**
		 * 设置地址
		 */
		void		setAddress( std::string host = "localhost", int port = DEFAULT_XASIO_PORT );
		
		/**
		 * 连接
		 */
		void		connect();
		/**
		 * 断开连接
		 */
		void		disconnect();
		
		/**
		 * 发送
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
		 * 主动接收
		 * 重载此函数可控制需要读取的方式，包头等等
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