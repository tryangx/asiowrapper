/**
 * 网络客户端封装
 */
#pragma once

#include "XAsioTCP.h"

namespace XGAME
{
	class XClient
	{
	public:
		/**
		 * 静态日志控制接口
		 */
		static void		setLog( std::function<void( const char* )> handler );
		static void		disableLog();

		/**
		 * 得到所有客户端发送的长度
		 */
		static size_t	getSendSize();
		/**
		 * 得到所有客户端接收的长度
		 */
		static size_t	getRecvSize();

	protected:
		static void		onLogHandler( const char* pLog );
				
	protected:
		static std::function<void( const char* )>	m_sfuncLogHandler;
		/**
		 * 所有客户端发送消息的长度
		 */
		static size_t		m_staSizeSend;
		/**
		 * 所有客户端接收消息的长度
		 */
		static size_t		m_staSizeRecv;

	public:
		XClient( XAsioService& io );
		~XClient();

		/**
		 * 获取服务接口
		 */
		TcpClientPtr	getService();
		
		/**
		 * 得到ID
		 */
		unsigned int	getId() const;
		void			setId( unsigned int id );

		bool			isConnected() const;

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
		void		send( std::string& content );
		void		send( XAsioBuffer& buff );

		/**
		 * 测试
		 */
		void		testSend();		
		void		sendThread();
		void		sendTestPackage();
		void		testEcho();

	public:
		template< typename HANDLER, typename OBJECT >
		void		setCloseHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcCloseHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }

	protected:
		/**
		 * 释放会话
		 */
		void		release();
		
		/**
		 * 主动接收
		 * 重载此函数可控制需要读取的方式，包头等等
		 */
		virtual void	recv();
		
		void		onConnect( TcpSessionPtr session );
		void		onRecv( XAsioBuffer& buff );
		void		onSend( size_t bytesTransferred );
		void		onResolve();
		void		onClose( size_t id );
		void		onLog( const char* pLog );
		
		void		onConnTimeoutCallback( const boost::system::error_code& ec );

	protected:
		/**
		 * asio服务对象
		 */
		XAsioService&		m_service;
		/**
		 * 网络接口，用于连接服务器
		 */
		TcpClientPtr		m_ptrTCPClient;
		/**
		 * 会话接口，用于收发消息
		 */
		TcpSessionPtr		m_ptrSession;

		/**
		 * 编号，用于池管理
		 */
		unsigned int		m_id;

		/**
		 * 连接信息
		 */
		int					m_iPort;
		std::string			m_sHost;

		/**
		 * 连接状态
		 */
		bool				m_bInit;
		bool				m_bIsConnected;

		/**
		 * 用于连接超时处理的计时器
		 */
		deadline_timer		m_connectTimer;

		/**
		 * 读取消息相关
		 */
		bool				m_bTestEcho;
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;
		XAsioPackage		m_lastRecvPackage;		

		/**
		 * 发送
		 */
		boost::shared_ptr<boost::thread>		m_sendThread;

		std::function<void( size_t )>			m_funcCloseHandler;
	};
}