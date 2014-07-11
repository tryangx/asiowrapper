#include "../../include/asio/XAsioUDP.h"
#include <boost/lexical_cast.hpp>

namespace XASIO
{
	UdpSessionPtr XAsioUDPSession::create( XAsioService& io )
	{
		return UdpSessionPtr( new XAsioUDPSession( io ) )->shared_from_this();
	}

	XAsioUDPSession::XAsioUDPSession( XAsioService& service ) : XAsioSessionInterface( service )
	{
		m_socket = UdpSocketPtr( new udp::socket( service ) );
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
		m_bufferSize = bufferSize;
		m_socket->async_receive( m_streamResponse.prepare( bufferSize ),
			boost::bind( &XAsioUDPSession::onRead, shared_from_this(), boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
	}
	void XAsioUDPSession::write( const XAsioBuffer& buffer )
	{
		std::ostream stream( &m_streamRequest );
		stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
		m_socket->async_send( m_streamRequest.data(), 
			boost::bind( &XAsioUDPSession::onWrite, shared_from_this(), boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred ) );
		m_streamRequest.consume( m_streamRequest.size() );
	}

	const UdpSocketPtr& XAsioUDPSession::getSocket() const { return m_socket; }

	//----------------------------------------

	UdpClientPtr XAsioUDPClient::create( XAsioService& io )
	{
		return UdpClientPtr( new XAsioUDPClient( io ) )->shared_from_this();
	}
	
	XAsioUDPClient::XAsioUDPClient( XAsioService& io )
		: XAsioClientInterface( io ), m_funcConnectHandler( nullptr ), m_resolver( nullptr )
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
		m_resolver = UdpResolverPtr( new udp::resolver( m_strand.get_io_service() ) );
		m_resolver->async_resolve( query, 
			m_strand.wrap( boost::bind( &XAsioUDPClient::onResolve, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
	}

	void XAsioUDPClient::setConnectHandler( const std::function< void( UdpSessionPtr ) >& eventHandler )
	{
		m_funcConnectHandler = eventHandler;
	}

	const UdpResolverPtr& XAsioUDPClient::getResolver() const
	{
		return  m_resolver;
	}

	void XAsioUDPClient::init()
	{
	}

	void XAsioUDPClient::release()
	{
	}

	void XAsioUDPClient::onConnect( UdpSessionPtr session, const boost::system::error_code& err )
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

	void XAsioUDPClient::onResolve( const boost::system::error_code& err, udp::resolver::iterator it )
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
				m_strand.wrap( boost::bind( &XAsioUDPClient::onConnect, 
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
		UdpSessionPtr session = XAsioUDPSession::create( m_service );
		boost::system::error_code err;
		session->getSocket()->open( boost::asio::ip::udp::v4(), err );
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
			return;
		}
		session->getSocket()->bind( udp::endpoint( udp::v4(), port ), err );
		if ( err ) 
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else if ( m_funcAcceptHandler != nullptr ) 
		{
			m_funcAcceptHandler( session );
		}
	}
}