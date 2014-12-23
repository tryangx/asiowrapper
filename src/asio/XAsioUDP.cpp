#include "asio/XAsioUDP.h"
#include "util/XStringUtil.h"
#include <boost/lexical_cast.hpp>

namespace XGAME
{
	XAsioUDPSession::XAsioUDPSession( XAsioServiceController& controller ) : XAsioSession( controller )
	{
		m_socket = UdpSocketPtr( new udp::socket( m_ioService ) );
	}

	XAsioUDPSession::~XAsioUDPSession()
	{
	}

	void XAsioUDPSession::recv()
	{
		recv( DEFAULT_READ_BYTES );
	}

	void XAsioUDPSession::recv( size_t bufferSize )
	{
		if ( bufferSize > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "read size is out of buffer length");
			return;
		}
		m_socket->async_receive( boost::asio::buffer( m_recvBuffer, bufferSize ),
			boost::bind( &XAsioUDPSession::onRecvCallback, shared_from_this(), boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
	}
	void XAsioUDPSession::send( XAsioBuffer& buffer )
	{
		size_t size = buffer.getDataSize();
		memcpy_s( (void*)m_sendBuffer, MAX_PACKET_SIZE, buffer.getData(), size );
		m_socket->async_send( boost::asio::buffer( m_sendBuffer, size ),
			boost::bind( &XAsioUDPSession::onSendCallback, shared_from_this(), boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred ) );
	}

	const UdpSocketPtr& XAsioUDPSession::getSocket() const { return m_socket; }

	//----------------------------------------
	
	XAsioUDPClient::XAsioUDPClient( XAsioServiceController& controller )
		: XAsioClientInterface( controller ), m_funcConnectHandler( nullptr ), m_ptrResolver( nullptr )
	{
	}

	XAsioUDPClient::~XAsioUDPClient()
	{
	}

	void XAsioUDPClient::connect( const std::string& host, uint16_t port )
	{
		connect( host, boost::lexical_cast<std::string>( port ) );
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
			XAsioLog::getInstance()->writeLog( "service:%d,code:%d,err:%s", m_dwServiceId, err.value(), err.message().c_str() );
		}
		else
		{
			UdpSessionPtr session( new XAsioUDPSession( m_controller ) );
			boost::asio::async_connect( *session->getSocket(), it, 
				m_strand.wrap( boost::bind( &XAsioUDPClient::onConnectCallback, 
				shared_from_this(), session, boost::asio::placeholders::error ) ) );
		}
	}

	//-----------------------------
	
	XAsioUDPServer::XAsioUDPServer( XAsioServiceController& controller )
		: XAsioServerInterface( controller ), m_funcAcceptHandler( nullptr )
	{
	}

	XAsioUDPServer::~XAsioUDPServer()
	{
		m_funcAcceptHandler = nullptr;
	}

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
			m_ptrSession = UdpSessionPtr( new XAsioUDPSession( m_controller ) )->shared_from_this();
		}
		boost::system::error_code err;
		m_ptrSession->getSocket()->open( boost::asio::ip::udp::v4(), err );
		if ( err ) 
		{
			XAsioLog::getInstance()->writeLog( "service:%d,code:%d %s", m_dwServiceId, err.value(), err.message().c_str() );
			return;
		}
		m_ptrSession->getSocket()->bind( udp::endpoint( udp::v4(), port ), err );
		if ( err ) 
		{
			XAsioLog::getInstance()->writeLog( "service:%d,code:%d %s", m_dwServiceId, err.value(), err.message().c_str() );
		}
		else
		{
			ON_CALLBACK_PARAM( m_funcAcceptHandler, m_ptrSession );
		}
	}

	void XAsioUDPClient::onConnectCallback( UdpSessionPtr session, const boost::system::error_code& err )
	{
		if ( err )
		{
			XAsioLog::getInstance()->writeLog( "service:%d,code:%d err:%s", m_dwServiceId, err.value(), err.message().c_str() );
		}
		else if ( m_funcConnectHandler != nullptr )
		{
			ON_CALLBACK_PARAM( m_funcConnectHandler, session );
		}
	}
}