#include "asio/XAsioTCP.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//---------------
	//TCP¿Í»§¶Ë¿ØÖÆ
	XAsioTCPClient::XAsioTCPClient( XAsioServiceController& controller )
		: XAsioClientInterface( controller ), m_funcConnectHandler( nullptr ), m_ptrResolver( nullptr )
	{
	}

	XAsioTCPClient::~XAsioTCPClient()
	{
		m_controller.removeService( this );

		if ( m_ptrSession )
		{
			m_ptrSession->close();
		}
	}

	void XAsioTCPClient::setConnectHandler( std::function<void( TcpSessionPtr )> handler )
	{
		m_funcConnectHandler = handler;
	}
	void XAsioTCPClient::setReconnectHandler( std::function<void()> handler )
	{
		m_funcReconnectHandler = handler;
	}

// 	void XAsioTCPClient::init()
// 	{
// 	}
// 
// 	void XAsioTCPClient::release()
// 	{
// 		m_funcConnectHandler	= nullptr;
// 		m_funcReconnectHandler	= nullptr;
// 	}

	void XAsioTCPClient::connect( const std::string& host, uint16_t port )
	{
		connect( host, boost::lexical_cast<std::string>( port ) );
	}
	void XAsioTCPClient::connect( const std::string& host, const std::string& protocol )
	{
		tcp::resolver::query query( host, protocol );
		if ( m_ptrResolver == nullptr )
		{
			m_ptrResolver = TcpResolverPtr( new tcp::resolver( m_strand.get_io_service() ) );
		}
		m_ptrResolver->async_resolve( query, 
			m_strand.wrap( boost::bind( &XAsioTCPClient::onResolveCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
	}

	TcpSessionPtr XAsioTCPClient::createTCPSession()
	{
		return TcpSessionPtr( new XAsioTCPSession( m_controller ) );
	}

	void XAsioTCPClient::onResolveCallback( const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator it )
	{
		if ( err )
		{
			if ( m_controller.getService( m_dwServiceId ) )
			{
				XAsioLog::getInstance()->writeLog( "service:%d,code:%d,%s", m_dwServiceId, err.value(), err.message().c_str() );
			}			
		}
		else
		{
			if ( m_ptrSession == nullptr )
			{
				m_ptrSession = createTCPSession();
			}
			boost::asio::async_connect( *m_ptrSession->getSocket(), it, 
				m_strand.wrap( boost::bind( &XAsioTCPClient::onConnectCallback, 
				shared_from_this(), m_ptrSession, boost::asio::placeholders::error ) ) );
		}
	}

	void XAsioTCPClient::onConnectCallback( TcpSessionPtr session, const boost::system::error_code& err )
	{
		if ( err )
		{
			if ( error::operation_aborted != err && getController().isRunning() )
			{
				if ( m_ptrSession )
				{
					m_ptrSession->close();
				}
			}
			if ( m_controller.getService( m_dwServiceId ) )
			{
				XAsioLog::getInstance()->writeLog( "service:%d,code:%d,%s", m_dwServiceId, err.value(), err.message().c_str() );
				ON_CALLBACK_PARAM( m_funcConnectHandler, nullptr );
				ON_CALLBACK( m_funcReconnectHandler );
			}
		}
		else 
		{
			ON_CALLBACK_PARAM( m_funcConnectHandler, session );
		}
	}
}