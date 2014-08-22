#include "asio/XAsioClient.h"
#include "asio/XAsioHelper.h"
#include "util/XStringUtil.h"
#include "asio/XAsioStat.h"

namespace XGAME
{
#define CONNECT_TIMEOUT_SECOND		10
	
	std::function<void( const char* )>	XClient::m_sfuncLogHandler = nullptr;

	void XClient::setLog( std::function<void( const char* )> handler ) { m_sfuncLogHandler = handler; }	
	void XClient::disableLog() { m_sfuncLogHandler = nullptr; }
	void XClient::onLogHandler( const char* pLog ) { if ( m_sfuncLogHandler != nullptr ) { m_sfuncLogHandler( pLog ); } }

	XClient::XClient( XAsioService& io ) : m_service( io ),
		m_iPort( DEFAULT_XASIO_PORT ), m_bInit( false ), m_bIsConnected( false ), m_id( 0 ),
		m_connectTimer( m_service.getIOService() ), m_ptrTCPClient( nullptr ), m_ptrSession( nullptr ),
		m_bReadHeader( false ), m_bTestEcho( false )
	{
	}

	XClient::~XClient()
	{
		disconnect();
		release();
	}

	TcpClientPtr XClient::getService() { return m_ptrTCPClient; }

	unsigned int XClient::getId() const { return m_id; }
	void XClient::setId( unsigned int id )
	{
		m_id = id;
		if ( m_ptrTCPClient )
		{
			m_ptrTCPClient->setId( id );
		}
	}

	bool XClient::isConnected() const
	{
		return m_bIsConnected;
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

	void XClient::connect()
	{
		if ( !m_bInit )
		{
			m_bInit = true;
			m_ptrTCPClient = XAsioTCPClient::create( m_service );
			setId( m_id );
		}
		if ( !m_bInit || m_bIsConnected )
		{
			onLog( "Client isn't initailized" );
			return;
		}
		m_ptrTCPClient->setConnectHandler( &XClient::onConnect, this );
		m_ptrTCPClient->setResolveHandler( &XClient::onResolve, this );
		m_ptrTCPClient->setLogHandler( &XClient::onLog, this );
		m_ptrTCPClient->connect( m_sHost, m_iPort );
		
		m_connectTimer.async_wait( boost::bind( &XClient::onConnTimeoutCallback, this, boost::asio::placeholders::error ) );

		m_bReadHeader = false;
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
				m_ptrSession->close();
			}
		}
		m_connectTimer.cancel();
	}
		
	void XClient::send( XAsioBuffer& buff )
	{
		if ( !m_bInit || !m_bIsConnected )
		{
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
	}

	void XClient::onConnect( TcpSessionPtr session )
	{
		if ( !m_bInit )
		{
			return;
		}
		if ( session == nullptr )
		{
			onLog( "cann't connec to server" );
			disconnect();
			return;
		}
		m_connectTimer.cancel();

		m_bIsConnected = true;
		
		m_ptrSession = session;
		m_ptrSession->setSessionId( m_id );
		m_ptrSession->setReadHandler( &XClient::onRecv, this );
		m_ptrSession->setWriteHandler( &XClient::onSend, this );
		m_ptrSession->setCloseHandler( &XClient::onClose, this );
		m_ptrSession->setLogHandler( &XClient::onLog, this );

		recv();

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
		ON_CALLBACK_PARAM( m_funcCloseHandler, m_id );
	}
	
	void XClient::onLog( const char* pLog )
	{
		std::string log = pLog;
		onLogHandler( outputString( "[%d]%s", m_id, log.c_str() ) );
	}
	
	void XClient::onConnTimeoutCallback( const boost::system::error_code& err )
	{
		if ( !m_bIsConnected )
		{
			if ( err )
			{
				onLog( err.message().c_str() );
			}
			else
			{
				disconnect();
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