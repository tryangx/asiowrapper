#pragma once

#include "XAsioInterface.h"
#include "XAsioSession.h"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

namespace XGAME
{
	enum enBufferType
	{
		EN_SESSION_SEND_BUFFER,
		EN_SESSION_RECV_BUFFER,

		EN_SESSION_BUFFER_COUNT	= 2,
		EN_SESSION_MUTEX_COUNT		= 2,
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
	
	/**
	 * TCP连接会话
	 */
	class XGAME_API XAsioTCPSession : public XAsioSession, public XAsioTimer, public boost::enable_shared_from_this<XAsioTCPSession>
	{
	public:
		XAsioTCPSession( XAsioServiceController& controller );
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
		virtual void	recv();

		/**
		 * 接收指定长度数据
		 */
		virtual void	recv( size_t bufferSize );

		/**
		 * 发送
		 */
		virtual void	send( XAsioBuffer& buffer );

		/**
		 * 暂停发送
		 * 
		 */
		void			suspendSend( bool b );
		/**
		 * 暂停接收数据的派发
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
		bool			processSend();
		/**
		 * 处理接收队列
		 */
		bool			processRead();

		/**
		 * 直接发送缓存
		 */
		void			sendImmediately( const XAsioBuffer& buffer );

		/**
		 * 定时器响应
		 */
		virtual bool	onTimer( unsigned int id, const void* pUserData );
		/**
		 * 完成接收响应处理
		 */
		virtual void	onRecvCallback( const boost::system::error_code& err, size_t bytesTransferred );
		/**
		 * 完成发送响应处理
		 */
		virtual void	onSendCallback( const boost::system::error_code& err, size_t bytesTransferred );
		/**
		 * 关闭的响应处理
		 */
		virtual void	onCloseCallback( const boost::system::error_code& err );

	protected:
		TcpSocketPtr			m_socket;
		
		typedef container::list<XAsioBuffer>	PACKET_CONAINER;
		/**
		 * 接收发送队列
		 */
		PACKET_CONAINER			m_packetBuffs[EN_SESSION_BUFFER_COUNT];		
		boost::mutex			m_mutexs[EN_SESSION_MUTEX_COUNT];

		/**
		 * 是否发送中标志
		 */
		bool					m_isSending;
		/**
		 * 是否暂停发送
		 */
		bool					m_isSuspendSend;
		/**
		 * 是否暂停派发
		 */
		bool					m_isSuspendDispatch;

		/**
		 * 发送数据长度
		 */
		unsigned long			m_dwSendSize;
		/**
		 * 接收数据长度
		 */
		unsigned long			m_dwRecvSize;
	};

	/**
	 * TCP客户端
	 */
	class XGAME_API XAsioTCPClient : public XAsioClientInterface, public boost::enable_shared_from_this<XAsioTCPClient>
	{
	public:
		XAsioTCPClient( XAsioServiceController& controller );
		~XAsioTCPClient();
		
		//virtual void	init();

		//virtual void	release();
		
		virtual void	connect( const std::string& host, uint16_t port );

		virtual void	connect( const std::string& host, const std::string& protocol );
		
		/**
		 * 设置连接到服务器成功的回调
		 */
		void			setConnectHandler( std::function<void( TcpSessionPtr )> );
		/**
		 * 设置触发重连时的回调
		 */
		void			setReconnectHandler( std::function<void()> );
		
	protected:				
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

	/**
	 * TCP服务器
	 */
	class XGAME_API XAsioTCPServer : public XAsioServerInterface, public boost::enable_shared_from_this<XAsioTCPServer>
	{
	public:
		XAsioTCPServer( XAsioServiceController& controller );
		~XAsioTCPServer();
		
		const TcpAcceptorPtr	getAcceptor() const;
				
		/**
		 * 开始侦听
		 * @param threadNum		侦听的线程数量
		 * @param port			侦听的端口
		 */
		virtual void	startAccept( int threadNum, unsigned short port );

		/**
		 * 开始侦听
		 * @param threadNum		侦听的线程数量
		 * @param port			侦听的端口
		 */
		virtual void	startAccept( unsigned short port );

		/**
		 * 停止侦听
		 */
		void			stopAccept();
		
	public:
		/**
		 * 侦听的响应处理函数
		 */
		void			setAcceptHandler( std::function<void( TcpSessionPtr )> handler );

		/**
		 * 停止侦听的响应处理函数
		 */
		void			setCancelHandler( std::function<void()> handler );
		
	protected:
		
		void			processAccept();

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