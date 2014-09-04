#include "asio/XAsioTCP.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//--------------------------------
	//TCP·þÎñÆ÷¿ØÖÆ
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
		stopAccept();
	}

	const TcpAcceptorPtr XAsioTCPServer::getAcceptor() const
	{
		return m_acceptor;
	}

	void XAsioTCPServer::init()
	{
	}

	void XAsioTCPServer::setAcceptHandler( std::function<void( TcpSessionPtr )> handler )
	{
		m_funcAcceptHandler = handler;
	}
	void XAsioTCPServer::setCancelHandler( std::function<void()> handler )
	{
		m_funcCancelHandler = handler;
	}

	void XAsioTCPServer::release()
	{
	}

	TcpSessionPtr XAsioTCPServer::createTCPSession()
	{
		return TcpSessionPtr( new XAsioTCPSession( m_service ) );
	}

	void XAsioTCPServer::startAccept( int threadNum, uint16_t port )
	{
		if ( m_acceptor == nullptr )
		{
			m_acceptor = TcpAcceptorPtr( new tcp::acceptor( m_ioService, tcp::endpoint( tcp::v4(), port) ) );
		}
		for ( int i = 0; i < threadNum; i++ )
		{
			startAccept();
		}
	}

	void XAsioTCPServer::startAccept()
	{
		TcpSessionPtr session = createTCPSession();
		m_acceptor->async_accept( *session->getSocket(), 
			m_strand.wrap( boost::bind( &XAsioTCPServer::onAcceptCallback, shared_from_this(), 
			session, boost::asio::placeholders::error ) ));
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
				ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "code:%d %s", err.value(), err.message().c_str() ) );
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
			try
			{
				if ( m_acceptor && m_acceptor->is_open() )
				{
					ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "code:%d %s", err.value(), err.message().c_str() ) );
				}				
			}
			catch(...)
			{
			}
		}
		else
		{
			ON_CALLBACK_PARAM( m_funcAcceptHandler, session );
			startAccept();
		}
	}
}