#include "asio/XAsioTCP.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//--------------------------------
	//TCP·þÎñÆ÷¿ØÖÆ
	XAsioTCPServer::XAsioTCPServer( XAsioServiceController& controller )
		: XAsioServerInterface( controller ), m_funcAcceptHandler( nullptr ), m_funcCancelHandler( nullptr ),
		m_acceptor( nullptr )
	{
	}

	XAsioTCPServer::~XAsioTCPServer()
	{
		m_controller.removeService( this );

		stopAccept();
	}

	const TcpAcceptorPtr XAsioTCPServer::getAcceptor() const
	{
		return m_acceptor;
	}

	void XAsioTCPServer::setAcceptHandler( std::function<void( TcpSessionPtr )> handler )
	{
		m_funcAcceptHandler = handler;
	}
	void XAsioTCPServer::setCancelHandler( std::function<void()> handler )
	{
		m_funcCancelHandler = handler;
	}

	TcpSessionPtr XAsioTCPServer::createTCPSession()
	{
		return TcpSessionPtr( new XAsioTCPSession( m_controller ) );
	}

	void XAsioTCPServer::startAccept( int threadNum, unsigned short port )
	{
		if ( m_acceptor == nullptr )
		{
			m_acceptor = TcpAcceptorPtr( new tcp::acceptor( m_ioService, tcp::endpoint( tcp::v4(), port) ) );
		}
		for ( int i = 0; i < threadNum; i++ )
		{
			processAccept();
		}
	}

	void XAsioTCPServer::startAccept( unsigned short port )
	{
		if ( m_acceptor == nullptr )
		{
			m_acceptor = TcpAcceptorPtr( new tcp::acceptor( m_ioService, tcp::endpoint( tcp::v4(), port ) ) );
		}
		processAccept();
	}
	
	void XAsioTCPServer::processAccept()
	{
		if ( m_acceptor )
		{
			TcpSessionPtr session = createTCPSession();
			m_acceptor->async_accept( *session->getSocket(), 
				m_strand.wrap( boost::bind( &XAsioTCPServer::onAcceptCallback, shared_from_this(), 
				session, boost::asio::placeholders::error ) ));
		}		
	}

	void XAsioTCPServer::stopAccept()
	{
		if ( m_acceptor )
		{
			boost::system::error_code err;			
			m_acceptor->cancel( err );
			m_acceptor->close( err );
			m_acceptor.reset();
			if ( err )
			{
				XAsioLog::getInstance()->writeLog( "service:%d,code:%d,%s", m_dwServiceId, err.value(), err.message().c_str() );
			}
			else
			{
				ON_CALLBACK( m_funcCancelHandler );
			}
		}
	}

	void XAsioTCPServer::onAcceptCallback( TcpSessionPtr session, const boost::system::error_code& err )
	{
		if ( err )
		{
			if ( m_controller.getService( m_dwServiceId ) )
			{
				if ( m_acceptor && m_acceptor->is_open() )
				{
					XAsioLog::getInstance()->writeLog( "service:%d,code:%d,%s", m_dwServiceId, err.value(), err.message().c_str() );
				}
			}
		}
		else
		{
			ON_CALLBACK_PARAM( m_funcAcceptHandler, session );
			processAccept();
		}
	}
}