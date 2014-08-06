#include "../../include/asio/XAsioUDP.h"
#include <boost/lexical_cast.hpp>

namespace XASIO
{
	UdpSessionPtr XAsioUDPSession::create( XAsioService& io )
	{
		return UdpSessionPtr( new XAsioUDPSession( io ) )->shared_from_this();
	}

	XAsioUDPSession::XAsioUDPSession( XAsioService& service ) : XAsioSession( service )
	{
		m_socket = UdpSocketPtr( new udp::socket( m_service.getIOService() ) );
	}

	XAsioUDPSession::~XAsioUDPSession()
	{
	}

	void XAsioUDPSession::init()
	{

	}
	void XAsioUDPSession::release()
	{

	}

	void XAsioUDPSession::read()
	{
		read( DEFAULT_READ_BYTES );
	}

	void XAsioUDPSession::read( size_t bufferSize )
	{
		if ( bufferSize > MAX_PACKAGE_LEN )
		{
			throw std::runtime_error( "read size is out of buffer length");
			return;
		}
		m_socket->async_receive( boost::asio::buffer( m_readBuffer, bufferSize ),
			boost::bind( &XAsioUDPSession::onReadCallback, shared_from_this(), boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
	}
	void XAsioUDPSession::write( XAsioBuffer& buffer )
	{
		size_t size = buffer.getDataSize();
		memcpy_s( (void*)m_sendBuffer, MAX_PACKAGE_LEN, buffer.getData(), size );
		m_socket->async_send( boost::asio::buffer( m_sendBuffer, size ),
			boost::bind( &XAsioUDPSession::onWriteCallback, shared_from_this(), boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred ) );
	}

	const UdpSocketPtr& XAsioUDPSession::getSocket() const { return m_socket; }

	//----------------------------------------

	UdpClientPtr XAsioUDPClient::create( XAsioService& io )
	{
		return UdpClientPtr( new XAsioUDPClient( io ) )->shared_from_this();
	}
	
	XAsioUDPClient::XAsioUDPClient( XAsioService& io )
		: XAsioClientInterface( io ), m_funcConnectHandler( nullptr ), m_ptrResolver( nullptr )
	{
	}

	XAsioUDPClient::~XAsioUDPClient()
	{
		m_funcConnectHandler = nullptr;
	}

	void XAsioUDPClient::connect( const std::string& host, uint16_t port )
	{
		//connect( host, boost::lexical_cast<std::string>( port ) );
	}

	void XAsioUDPClient::connect( const std::string& host, const std::string& protocol )
	{
		udp::resolver::query query( host, protocol );
		if ( m_ptrResolver != nullptr )
		{
			m_ptrResolver = UdpResolverPtr( new udp::resolver( m_strand.get_io_service() ) );
		}
		m_ptrResolver->async_resolve( query, 
			m_strand.wrap( boost::bind( &XAsioUDPClient::onResolveCallback, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
	}

	void XAsioUDPClient::setConnectHandler( const std::function< void( UdpSessionPtr ) >& eventHandler )
	{
		m_funcConnectHandler = eventHandler;
	}

	void XAsioUDPClient::init()
	{
	}

	void XAsioUDPClient::release()
	{
	}
	
	void XAsioUDPClient::onResolveCallback( const boost::system::error_code& err, udp::resolver::iterator it )
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
			UdpSessionPtr session( new XAsioUDPSession( m_service ) );
			boost::asio::async_connect( *session->getSocket(), it, 
				m_strand.wrap( boost::bind( &XAsioUDPClient::onConnectCallback, 
				shared_from_this(), session, boost::asio::placeholders::error ) ) );
		}
	}

	//-----------------------------

	UdpServerPtr XAsioUDPServer::create( XAsioService& io )
	{
		return UdpServerPtr( new XAsioUDPServer( io ) )->shared_from_this();
	}

	XAsioUDPServer::XAsioUDPServer( XAsioService& io )
		: XAsioServerInterface( io ), m_funcAcceptHandler( nullptr )
	{
	}

	XAsioUDPServer::~XAsioUDPServer() { m_funcAcceptHandler = nullptr; }

	void XAsioUDPServer::setAcceptHandler( const std::function<void( UdpSessionPtr )>& eventHandler )
	{
		m_funcAcceptHandler = eventHandler;
	}

	void XAsioUDPServer::init()
	{

	}
	void XAsioUDPServer::release()
	{

	}

	void XAsioUDPServer::startAccept( int threadNum, uint16_t port )
	{
		if ( m_ptrSession != nullptr )
		{
			m_ptrSession = UdpSessionPtr( new XAsioUDPSession( m_service ) )->shared_from_this();
		}
		boost::system::error_code err;
		m_ptrSession->getSocket()->open( boost::asio::ip::udp::v4(), err );
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
			return;
		}
		m_ptrSession->getSocket()->bind( udp::endpoint( udp::v4(), port ), err );
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else if ( m_funcAcceptHandler != nullptr ) 
		{
			m_funcAcceptHandler( m_ptrSession );
		}
	}

	void XAsioUDPClient::onConnectCallback( UdpSessionPtr session, const boost::system::error_code& err )
	{
		if ( err )
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else if ( m_funcConnectHandler != nullptr )
		{
			m_funcConnectHandler( session );
		}
	}
}