#include "asio/XAsioClient.h"
#include "asio/XAsioHelper.h"
#include "util/XStringUtil.h"
#include "asio/XAsioStat.h"

namespace XGAME
{
#define DEFAULT_CONNECT_TIMEOUT_MS			10000

#define DEFAULT_DISCONNECT_TIMEOUT_MS		6000
	
	XClient::XClient( XAsioService& io ) : m_service( io ),
		m_iPort( 6580 ), m_bInit( false ), m_bIsConnected( false ), m_iClientId( 0 ),
		m_timer( m_service.getIOService() ), m_ptrTCPClient( nullptr ), m_ptrSession( nullptr ),
		m_bReadHeader( false ), m_bTestEcho( false )
	{
	}

	XClient::~XClient()
	{
		disconnect();
		release();
		
		m_funcCloseHandler		=	nullptr;
		m_funcRecvHandler		=	nullptr;
		m_funcConnectHandler	=	nullptr;
		m_funcLogHandler		=	nullptr;
	}

	TcpClientPtr XClient::getServicePtr() { return m_ptrTCPClient; }
	XAsioTCPClient*	XClient::getService() { return m_ptrTCPClient ? m_ptrTCPClient.get() : NULL; }

	unsigned int XClient::getClientId() const { return m_iClientId; }
	void XClient::setClientId( unsigned int id )
	{
		m_iClientId = id;
		if ( m_ptrTCPClient )
		{
			m_ptrTCPClient->setServiceId( id );
		}
	}

	bool XClient::isConnected() const
	{
		return m_bIsConnected;
	}
	
	void XClient::setConnectHandler( std::function<void( XClient* )> handler )
	{
		m_funcConnectHandler = handler;
	}
	void XClient::setCloseHandler( std::function<void( size_t )> handler )
	{
		m_funcCloseHandler = handler;
	}
	void XClient::setRecvHandler( std::function<void( XClient*, XAsioRecvPacket& )> handler )
	{
		m_funcRecvHandler = handler;
	}
	void XClient::setLogHandler( std::function<void( const char* )> handler )
	{
		m_funcLogHandler = handler;
	}
	const char*	XClient::getIp() const
	{
		return m_sHost.c_str();
	}
	unsigned short XClient::getPort() const
	{
		return m_iPort;
	}
	void XClient::setAddress( std::string host, int port )
	{
		if ( m_bIsConnected )
		{
			onLog( "Client already connect to server!" );
			return;
		}
		m_sHost = host;
		m_iPort = port;
	}

	bool XClient::connect()
	{
		if ( m_bIsConnected )
		{
			onLog( "Client is already connected!" );
			return false;
		}
		m_bInit = true;
		m_ptrTCPClient.reset();
		m_ptrTCPClient = XAsioTCPClient::create( m_service );
		m_ptrTCPClient->setConnectHandler( std::bind( &XClient::onConnect, this, std::placeholders::_1 ) );
		m_ptrTCPClient->setResolveHandler( std::bind( &XClient::onResolve, this ) );
		m_ptrTCPClient->setLogHandler( std::bind( &XClient::onLog, this, std::placeholders::_1 ) );
		m_ptrTCPClient->connect( m_sHost, m_iPort );
		setClientId( m_iClientId );
		
		m_timer.expires_from_now( posix_time::millisec( DEFAULT_CONNECT_TIMEOUT_MS ) );
		m_timer.async_wait( boost::bind( &XClient::onConnTimeoutCallback, this, boost::asio::placeholders::error ) );

		m_bReadHeader = false;
		return true;
	}

	void XClient::release()
	{
		if ( m_ptrSession )
		{
			m_ptrSession->release();			
		}
		m_bIsConnected = false;
	}

	void XClient::disconnect()
	{
		if ( m_bIsConnected )
		{
			if ( m_ptrSession )
			{
				m_ptrSession->release();
				m_ptrSession->close();
			}
		}
		m_timer.cancel();
	}
		
	void XClient::send( XAsioBuffer& buff )
	{
		if ( !m_bIsConnected )
		{
			onLog( "Cann't send packet without connect to sever" );
			return;
		}
		if ( m_ptrSession && m_ptrSession->isOpen() )
		{
			m_ptrSession->write( buff );
		}
	}

	void XClient::recv()
	{
		if ( !isConnected() )
		{
			return;
		}
		if ( m_bTestEcho )
		{
			m_ptrSession->read( XAsioPacketHeader::getHeaderSize() );
		}
		else
		{
			if ( m_recvPacket.isEmpty() )
			{
				m_ptrSession->read( XAsioPacketHeader::getHeaderSize() );
			}
			else if ( m_recvPacket.getHeader() )
			{
				m_ptrSession->read( m_recvPacket.getHeader()->m_dwSize );
			}
		}
//		m_timer.expires_from_now( posix_time::millisec( DEFAULT_CONNECT_TIMEOUT_MS ) );
//		m_timer.async_wait( boost::bind( &XClient::onDisconnTimeCallback, this, boost::asio::placeholders::error ) );
	}

	void XClient::onConnect( TcpSessionPtr session )
	{
		if ( !m_bInit )
		{
			return;
		}
		if ( session == nullptr )
		{
			onLog( "cann't connect to server" );
			disconnect();
			return;
		}
		m_timer.cancel();
		
		m_bIsConnected = true;
		
		m_ptrSession = session;
		m_ptrSession->setSessionId( m_iClientId );
		m_ptrSession->setReadHandler( std::bind( &XClient::onRecv, this, std::placeholders::_1 ) );
		m_ptrSession->setWriteHandler( std::bind( &XClient::onSend, this, std::placeholders::_1 ) );
		m_ptrSession->setCloseHandler( std::bind( &XClient::onClose, this, std::placeholders::_1 ) );
		m_ptrSession->setLogHandler( std::bind( &XClient::onLog, this, std::placeholders::_1 ) );

		recv();

		ON_CALLBACK_PARAM( m_funcConnectHandler, this );
		//onLog( "Client connect server!" );
	}

	void XClient::onRecv( XAsioBuffer& buff )
	{
		XAsioStatClientAgent::getMutableInstance()->recv( buff.getDataSize() );
		m_recvPacket.import( buff );
		if ( m_recvPacket.isReady() )
		{
			if ( m_recvPacket.getHeader()->m_cOp == EN_POP_ECHO )
			{
				send( buff );
			}
			ON_CALLBACK_PARAM2( m_funcRecvHandler, this, m_recvPacket );
		}
		recv();		
	}


	void XClient::onSend( size_t bytesTransferred )
	{
		XAsioStatClientAgent::getMutableInstance()->send( bytesTransferred );
	}

	void XClient::onResolve()
	{
	}

	void XClient::onClose( size_t id )
	{
		m_bIsConnected = false;
		ON_CALLBACK_PARAM( m_funcCloseHandler, m_iClientId );		
	}
	
	void XClient::onLog( const char* pLog )
	{
		std::string log = pLog;
		ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "[%d]%s", m_iClientId, log.c_str() ) );
	}
	
	void XClient::onConnTimeoutCallback( const boost::system::error_code& err )
	{
		if ( !m_bIsConnected )
		{
			if ( !err )
			{
				ON_CALLBACK_PARAM( m_funcConnectHandler, NULL );
			}
		}
	}

	void XClient::onDisconnTimeCallback( const boost::system::error_code& err )
	{
		if ( m_bIsConnected )
		{
			if ( !err )
			{
				ON_CALLBACK_PARAM( m_funcCloseHandler, NULL );
			}
		}
	}

	void XClient::testEcho()
	{
		if ( m_bTestEcho )
		{
			return;
		}
		m_bTestEcho = true;

		XAsioPacketHeader header;
		header.m_dwType = 1;
		header.m_dwSize = 999;
		
		recv();

		XAsioBuffer buff;
		buff.copy( &header, sizeof(header) );
		send( buff );
	}
}