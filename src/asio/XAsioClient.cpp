#include "asio/XAsioClient.h"
#include "asio/XAsioHelper.h"
#include "util/XStringUtil.h"
#include "asio/XAsioStat.h"

namespace XGAME
{
	//默认的连接超时判定时间
#define DEFAULT_CONNECT_TIMEOUT_MS			10000

	//默认的超时断开判定时间
#define DEFAULT_DISCONNECT_TIMEOUT_MS		6000
	
	XClient::XClient( XAsioServiceController& controller ) : m_controller( controller ),
		m_iPort( DEFAULT_PORT ), m_bIsConnected( false ), m_bTimeoutDisconnect( false ), m_dwClientId( 0 ),
		m_timer( controller.getAsioIOService() ), m_ptrTCPClient( nullptr ), m_ptrSession( nullptr ),
		m_bReadHeader( false ), m_bTestEcho( false )
	{
	}

	XClient::~XClient()
	{
		disconnect();
		
		if ( m_ptrSession )
		{
			m_ptrSession->release();			
		}
		m_bIsConnected = false;
	}

	XAsioTCPClient*	XClient::getService()
	{
		return m_ptrTCPClient ? m_ptrTCPClient.get() : NULL;
	}

	unsigned long XClient::getClientId() const
	{
		return m_dwClientId;
	}

	void XClient::setClientId( unsigned long id )
	{
		m_dwClientId = id;
		if ( m_ptrTCPClient )
		{
			m_ptrTCPClient->setServiceId( id );
		}
	}

	bool XClient::isConnected() const
	{
		return m_bIsConnected;
	}
	
	void XClient::setTimeoutDisconnect( bool enable )
	{
		m_bTimeoutDisconnect = enable;
	}

	void XClient::setConnectHandler( std::function<void( XClient* )> handler )
	{
		m_funcConnectHandler = handler;
	}
	void XClient::setCloseHandler( std::function<void( unsigned long )> handler )
	{
		m_funcCloseHandler = handler;
	}
	void XClient::setRecvHandler( std::function<void( XClient*, XAsioRecvPacket& )> handler )
	{
		m_funcRecvHandler = handler;
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
		m_ptrTCPClient.reset();
		m_ptrTCPClient = TcpClientPtr( new XAsioTCPClient( m_controller ) );
		m_ptrTCPClient->setConnectHandler( std::bind( &XClient::onConnect, this, std::placeholders::_1 ) );
		m_ptrTCPClient->connect( m_sHost, m_iPort );
		setClientId( m_dwClientId );
		
		m_timer.expires_from_now( posix_time::millisec( DEFAULT_CONNECT_TIMEOUT_MS ) );
		m_timer.async_wait( boost::bind( &XClient::onConnTimeoutCallback, this, boost::asio::placeholders::error ) );

		m_bReadHeader = false;
		return true;
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
			m_ptrSession->send( buff );
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
			m_ptrSession->recv( XAsioPacketHeader::getHeaderSize() );
		}
		else
		{
			if ( m_recvPacket.isEmpty() )
			{
				m_ptrSession->recv( XAsioPacketHeader::getHeaderSize() );
			}
			else if ( m_recvPacket.getHeader() )
			{
				m_ptrSession->recv( m_recvPacket.getHeader()->m_dwSize );
			}
		}

		if ( m_bTimeoutDisconnect )
		{
			m_timer.expires_from_now( posix_time::millisec( DEFAULT_DISCONNECT_TIMEOUT_MS ) );
			m_timer.async_wait( boost::bind( &XClient::onDisconnTimeCallback, this, boost::asio::placeholders::error ) );
		}
	}

	void XClient::onConnect( TcpSessionPtr session )
	{
		if ( session == nullptr )
		{
			onLog( "cann't connect to server" );
			disconnect();
			return;
		}
		m_timer.cancel();
		
		m_bIsConnected = true;
		
		m_ptrSession = session;
		m_ptrSession->setSessionId( m_dwClientId );
		m_ptrSession->setRecvHandler( std::bind( &XClient::onRecv, this, std::placeholders::_1 ) );
		m_ptrSession->setSendHandler( std::bind( &XClient::onSend, this, std::placeholders::_1 ) );
		m_ptrSession->setCloseHandler( std::bind( &XClient::onClose, this, std::placeholders::_1 ) );

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

	void XClient::onClose( size_t id )
	{
		m_bIsConnected = false;
		ON_CALLBACK_PARAM( m_funcCloseHandler, m_dwClientId );		
	}
	
	void XClient::onLog( const char* pLog )
	{
		XAsioLog::getInstance()->writeLog( "[%d]%s", m_dwClientId, pLog );
	}
	
	void XClient::onConnTimeoutCallback( const boost::system::error_code& err )
	{
		if ( !m_bIsConnected && !err )
		{
			ON_CALLBACK_PARAM( m_funcConnectHandler, NULL );
		}
	}

	void XClient::onDisconnTimeCallback( const boost::system::error_code& err )
	{
		if ( m_bIsConnected && !err )
		{
			ON_CALLBACK_PARAM( m_funcCloseHandler, NULL );
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