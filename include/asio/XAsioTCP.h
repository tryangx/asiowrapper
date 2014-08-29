#pragma once

#include "XAsioInterface.h"
#include "XAsioSession.h"
#include "../util/XLog.h"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

namespace XGAME
{
	enum enBufferType
	{
		SESSION_SEND_BUFFER,
		SESSION_RECV_BUFFER,

		SESSION_BUFFER_COUNT	= 2,
		SESSION_MUTEX_COUNT		= 2,
	};

	enum enTimerID
	{
		SESSION_DISPATCH_TIMERID,
	};

	using namespace boost::asio::ip;

	class XAsioTCPSession;
	class XAsioTCPClient;
	class XAsioTCPServer;
	
	typedef boost::shared_ptr<class XAsioTCPSession>			TcpSessionPtr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::socket>		TcpSocketPtr;
	
	typedef boost::shared_ptr<class XAsioTCPClient>				TcpClientPtr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::resolver>	TcpResolverPtr;
			
	typedef boost::shared_ptr<class XAsioTCPServer>				TcpServerPtr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor>	TcpAcceptorPtr;
	
	//----------------------
	//	TCP连接会话
	class XGAME_API XAsioTCPSession : public XAsioSession, public XAsioTimer, public boost::enable_shared_from_this<XAsioTCPSession>
	{
	public:
		static TcpSessionPtr	create( XAsioService& io );

	public:
		XAsioTCPSession( XAsioService& io );
		~XAsioTCPSession();

		/**
		 * 获取套接字
		 */
		const TcpSocketPtr	getSocket() const;
		
		/**
		 * 是否打开
		 */
		bool			isOpen();

		/**
		 * 关闭会话
		 */
		void			close();

		/**
		 * 接收数据
		 */
		virtual void	read();

		/**
		 * 接收指定长度数据
		 */
		virtual void	read( size_t bufferSize );

		/**
		 * 发送
		 */
		virtual void	write( XAsioBuffer& buffer );

		/**
		 * 发送挂起
		 */
		void			suspendSend( bool b );
		/**
		 * 派发挂起
		 */
		void			suspendDispatch( bool b );
			
		/**
		 * 获取已发送字节
		 */
		size_t			getSendSize() const;
		/**
		 * 获取已接收字节
		 */
		size_t			getRecvSize() const;

	protected:
		/**
		 * 处理发送队列
		 */
		bool			doSend();
		/**
		 * 处理接收队列
		 */
		bool			doRead();

		/**
		 * 直接发送缓存
		 */
		void			sendDirectly( const XAsioBuffer& buffer );

		virtual bool	onTimer( unsigned int id, const void* pUserData );
		virtual void	onReadCallback( const boost::system::error_code& err, size_t bytesTransferred );
		virtual void	onWriteCallback( const boost::system::error_code& err, size_t bytesTransferred );

		/**
		 * 响应关闭的处理
		 */
		virtual void	onCloseCallback( const boost::system::error_code& err );

	protected:
		TcpSocketPtr			m_socket;
		
		typedef container::list<XAsioBuffer>	PACKAGE_CONAINER;
		PACKAGE_CONAINER		m_buffers[SESSION_BUFFER_COUNT];
		boost::mutex			m_mutexs[SESSION_MUTEX_COUNT];

		bool					m_isSending;
		bool					m_isSuspendSend;
		bool					m_isSuspendDispatch;

		size_t					m_sendSize;
		size_t					m_recvSize;
	};

	//-----------------------
	//	TCP客户端
	class XGAME_API XAsioTCPClient : public XAsioClientInterface, public boost::enable_shared_from_this<XAsioTCPClient>
	{
	public:
		static TcpClientPtr	create( XAsioService& io );

	public:
		~XAsioTCPClient();
		
		virtual void	init();

		virtual void	release();
		
		/**
		 * 连接
		 */
		virtual void	connect( const std::string& host, uint16_t port );
		virtual void	connect( const std::string& host, const std::string& protocol );
		
	public:
		void			setConnectHandler( std::function<void( TcpSessionPtr )> );
		void			setReconnectHandler( std::function<void()> );
		
	protected:
		XAsioTCPClient( XAsioService& io );
				
		/**
		 * 生成线程对象
		 * 重载可用于内存池管理
		 */
		virtual	TcpSessionPtr	createTCPSession();
		
		/**
		 * 搜索到连接的响应
		 */
		virtual void	onResolveCallback( const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator it );
		
		/**
		 * 连接时的响应
		 */
		virtual void	onConnectCallback( TcpSessionPtr session, const boost::system::error_code& err );
				
	protected:
		TcpResolverPtr							m_ptrResolver;
		TcpSessionPtr							m_ptrSession;

		std::function<void( TcpSessionPtr )>	m_funcConnectHandler;
		std::function<void()>					m_funcReconnectHandler;
	};

	//-----------------------
	//	TCP server
	class XGAME_API XAsioTCPServer : public XAsioServerInterface, public boost::enable_shared_from_this<XAsioTCPServer>
	{
	public:
		static TcpServerPtr	create( XAsioService& io );

	public:
		~XAsioTCPServer();
		
		const TcpAcceptorPtr	getAcceptor() const;

		virtual void	init();
		virtual void	release();
		
		/**
		 * 开始侦听
		 * @param threadNum		侦听的线程数量
		 * @param port			侦听的端口
		 */
		virtual void	startAccept( int threadNum, uint16_t port );
		virtual void	startAccept();

		/**
		 * 停止侦听
		 */
		void			stopAccept();
		
	public:
		/**
		 * 侦听的响应处理函数
		 * 函数原型为void handler( XAsioTCPSession session ) 
		 */
		void			setAcceptHandler( std::function<void( TcpSessionPtr )> handler );

		/**
		 * 停止侦听的响应处理函数
		 * 函数原型为void handler( XAsioTCPSession session ) 
		 */
		void			setCancelHandler( std::function<void()> handler );
		
	protected:
		XAsioTCPServer( XAsioService& io );
				
		/**
		 * 生成线程对象
		 * 重载可用于内存池管理
		 */
		virtual	TcpSessionPtr	createTCPSession();

		/**
		 * 响应侦听的连接
		 */
		void			onAcceptCallback( TcpSessionPtr session, const boost::system::error_code& err );

	protected:
		TcpAcceptorPtr							m_acceptor;

		std::function<void( TcpSessionPtr )>	m_funcAcceptHandler;
		std::function<void()>					m_funcCancelHandler;
	};
}