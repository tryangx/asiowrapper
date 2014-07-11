#include "../include/XAsioTCP.h"

namespace XASIO
{
	//-----------------------------
	//	TCP连接
	TcpSessionPtr XAsioTCPSession::create( XAsioService& io )
	{
		return TcpSessionPtr( new XAsioTCPSession( io ) )->shared_from_this();
	}
	
	XAsioTCPSession::XAsioTCPSession( XAsioService& io )
		: XAsioSessionInterface( io )
	{
		m_socket = TcpSocketPtr( new tcp::socket( io ) );
	}

	XAsioTCPSession::~XAsioTCPSession()
	{
		close();
	}
	
	const TcpSocketPtr XAsioTCPSession::getSocket() const { return m_socket; }

	void XAsioTCPSession::close()
	{
		if ( m_socket && m_socket->is_open() ) 
		{
			boost::system::error_code err;
			m_socket->shutdown( tcp::socket::shutdown_both, err );
			m_socket->close( err );
			onClose( err );
		}
	}

	void XAsioTCPSession::read()
	{
		boost::asio::async_read( *m_socket, m_streamResponse, boost::asio::transfer_at_least( 1 ),
			boost::bind( &XAsioTCPSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
	}
	void XAsioTCPSession::read( const std::string& delim )
	{
		boost::asio::async_read_until( *m_socket, m_streamResponse, delim, 
			m_strand.wrap( boost::bind( &XAsioTCPSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}
	void XAsioTCPSession::read( size_t bufferSize )
	{
		m_socket->async_read_some( m_streamResponse.prepare( bufferSize ), 
			m_strand.wrap( boost::bind( &XAsioTCPSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}
	void XAsioTCPSession::write( const XAsioBuffer& buffer )
	{
		std::ostream stream( &m_streamRequest );
		stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
		boost::asio::async_write( *m_socket, m_streamRequest, 
			m_strand.wrap( boost::bind( &XAsioTCPSession::onWrite, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
		m_streamRequest.consume( m_streamRequest.size() );
	}

	void XAsioTCPSession::onClose( const boost::system::error_code& err )
	{
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else if ( m_funcCloseHandler != nullptr )
		{
			m_funcCloseHandler( getId() );
		}
	}

	//---------------
	//TCP客户端控制
	TcpClientPtr XAsioTCPClient::create( XAsioService& io )
	{
		return TcpClientPtr( new XAsioTCPClient( io ) )->shared_from_this();
	}

	XAsioTCPClient::XAsioTCPClient( XAsioService& io )
		: XAsioClientInterface( io ), m_funcConnectHandler( nullptr ), m_ptrResolver( nullptr )
	{
	}

	XAsioTCPClient::~XAsioTCPClient()
	{
		m_funcConnectHandler = nullptr;
	}

	TcpResolverPtr XAsioTCPClient::getResolver() const { return m_ptrResolver; }

	void XAsioTCPClient::init()
	{
	}

	void XAsioTCPClient::release()
	{
		m_funcLogHandler		= nullptr;
		m_funcConnectHandler	= nullptr;
		m_funcResolveHandler	= nullptr;
	}

	void XAsioTCPClient::connect( const std::string& host, uint16_t port )
	{
		connect( host, boost::lexical_cast<std::string>( port ) );
	}
	void XAsioTCPClient::connect( const std::string& host, const std::string& protocol )
	{
		tcp::resolver::query query( host, protocol );
		m_ptrResolver = TcpResolverPtr( new tcp::resolver( m_strand.get_io_service() ) );
		m_ptrResolver->async_resolve( query, 
			m_strand.wrap( boost::bind( &XAsioTCPClient::onResolve, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
	}

	void XAsioTCPClient::onResolve( const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator it )
	{
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else
		{
			if ( m_funcResolveHandler != nullptr ) 
			{
				m_funcResolveHandler();
			}
			//TcpSessionPtr session( new XAsioTCPSession( m_service ) );
			m_ptrSession = TcpSessionPtr( new XAsioTCPSession( m_service ) )->shared_from_this();
			boost::asio::async_connect( *m_ptrSession->getSocket(), it, 
				m_strand.wrap( boost::bind( &XAsioTCPClient::onConnect, 
				shared_from_this(), m_ptrSession, boost::asio::placeholders::error ) ) );
		}
	}

	void XAsioTCPClient::onConnect( TcpSessionPtr session, const boost::system::error_code& err )
	{
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else 
		{
			if ( m_funcConnectHandler != nullptr ) 
			{
				m_funcConnectHandler( session );
			}
		}
	}
	
	//--------------------------------
	//TCP服务器控制
	TcpServerPtr XAsioTCPServer::create( XAsioService& io )
	{
		return TcpServerPtr( new XAsioTCPServer( io ) )->shared_from_this();
	}
	
	XAsioTCPServer::XAsioTCPServer( XAsioService& io )
		: XAsioServerInterface( io ), m_funcAcceptHandler( nullptr ), m_funcCancelHandler( nullptr ),
		m_acceptor( nullptr )
	{
	}

	XAsioTCPServer::~XAsioTCPServer()
	{
		m_funcAcceptHandler		= nullptr;
		m_funcCancelHandler		= nullptr;

		stopAccept();
	}

	const TcpAcceptorPtr XAsioTCPServer::getAcceptor() const
	{
		return m_acceptor;
	}

	void XAsioTCPServer::init()
	{
	}

	void XAsioTCPServer::release()
	{
	}

	void XAsioTCPServer::startAccept( int threadNum, uint16_t port )
	{
		m_acceptor = TcpAcceptorPtr( new tcp::acceptor( m_service, tcp::endpoint( tcp::v4(), port) ) );
		for ( int i = 0; i < threadNum; i++ )
		{
			startAccept();
		}
	}

	void XAsioTCPServer::startAccept()
	{
		TcpSessionPtr session( new XAsioTCPSession( m_service ) );
		m_acceptor->async_accept( *session->getSocket(), 
			m_strand.wrap( boost::bind( &XAsioTCPServer::onAccept, shared_from_this(), 
			session, boost::asio::placeholders::error ) ));
	}

	void XAsioTCPServer::stopAccept()
	{
		if ( m_acceptor )
		{
			boost::system::error_code err;
			m_acceptor->cancel( err );
			m_acceptor->close( err );
			if ( err )
			{
				if ( m_funcLogHandler != nullptr )
				{
					m_funcLogHandler( err.message() );
				}
			}
			else if ( m_funcCancelHandler != nullptr )
			{
				m_funcCancelHandler();
			}
		}
	}

	void XAsioTCPServer::onAccept( TcpSessionPtr session, const boost::system::error_code& err )
	{
		if ( err )
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else
		{
			if ( m_funcAcceptHandler != nullptr ) 
			{
				m_funcAcceptHandler( session );
			}
			startAccept();
		}
	}
}