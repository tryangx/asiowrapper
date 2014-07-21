#pragma once

#include "XAsioInterface.h"
#include "../util/XLog.h"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

namespace XASIO
{
#define DEFAULT_XASIO_PORT		7777

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
	class XAsioTCPSession : public XAsioSessionInterface, public boost::enable_shared_from_this<XAsioTCPSession>
	{
	public:
		static TcpSessionPtr	create( XAsioService& io );

	public:
		XAsioTCPSession( XAsioService& io );
		~XAsioTCPSession();

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
		 * 读取
		 */
		virtual void	read();
		virtual	void	read( const std::string& delim );
		virtual void	read( size_t bufferSize );

		/**
		 * 发送
		 */
		virtual void	write( const XAsioBuffer& buffer );
			
	protected:
		/**
		 * 响应关闭的处理
		 */
		virtual void	onCloseCallback( const boost::system::error_code& err );

	protected:
		TcpSocketPtr					m_socket;
	};

	//-----------------------
	//	TCP客户端
	class XAsioTCPClient : public XAsioClientInterface, public boost::enable_shared_from_this<XAsioTCPClient>
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
		template< typename HANDLER, typename OBJECT >
		void			setConnectHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcConnectHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		XAsioTCPClient( XAsioService& io );
		
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
	};

	//-----------------------
	//	TCP server
	class XAsioTCPServer : public XAsioServerInterface, public boost::enable_shared_from_this<XAsioTCPServer>
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
		template< typename HANDLER, typename OBJECT >
		void			setAcceptHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcAcceptHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }

		/**
		 * 停止侦听的响应处理函数
		 * 函数原型为void handler( XAsioTCPSession session ) 
		 */
		template< typename HANDLER, typename OBJECT >
		void			setCancelHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcCancelHandler = std::bind( eventHandler, eventHandlerObject ); }
		
	protected:
		XAsioTCPServer( XAsioService& io );

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