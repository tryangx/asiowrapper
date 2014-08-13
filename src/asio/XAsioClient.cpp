#include "../../include/asio/XAsioClient.h"
#include "../../include/asio/XAsioHelper.h"

namespace XASIO
{
#define CONNECT_TIMEOUT_SECOND		10

	size_t XClient::m_staSizeSend = 0;
	size_t XClient::m_staSizeRecv = 0;

	std::function<void( const char* )>	XClient::m_sfuncLogHandler = nullptr;

	void XClient::setLog( std::function<void( const char* )> handler ) { m_sfuncLogHandler = handler; }	
	void XClient::disableLog() { m_sfuncLogHandler = nullptr; }
	void XClient::onLogHandler( const char* pLog ) { if ( m_sfuncLogHandler != nullptr ) { m_sfuncLogHandler( pLog ); } }

	size_t XClient::getSendSize() { return m_staSizeSend; }
	size_t XClient::getRecvSize() { return m_staSizeRecv; }

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
			//onLog( "disconnect" );		//will cause lock
		}
		m_connectTimer.cancel();

		if ( m_sendThread )
		{
			m_sendThread->interrupt();
		}		
	}

	void XClient::testSend()
	{
		if ( m_sendThread == nullptr )
		{
			m_sendThread = boost::shared_ptr<boost::thread>( new boost::thread( boost::bind( &XClient::sendThread, this ) ) );
		}
	}
	
	void XClient::send( std::string& content )
	{
		if ( !m_bInit || !m_bIsConnected )
		{
			return;
		}
		if ( m_ptrSession && m_ptrSession->isOpen() )
		{
			XAsioBuffer buff;
			buff.convertFromString( content );
			m_ptrSession->write( buff );
		}
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
			m_ptrSession->read( XAsioPackageHeader::getHeaderSize() );
		}
		else
		{
			if ( m_bReadHeader && m_packageHeader.m_dwSize > 0 )
			{
				if ( m_packageHeader.m_dwSize != sizeof(XAsioPackage) )
				{
					onLog( outputString( "FATAL ERROR %d", m_packageHeader.m_dwSize ) );
				}
				m_ptrSession->read( m_packageHeader.m_dwSize );
			}
			else
			{
				m_ptrSession->read( XAsioPackageHeader::getHeaderSize() );
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

		onLog( "Client connect server!" );
	}

	void XClient::onRecv( XAsioBuffer& buff )
	{
		if ( m_bTestEcho )
		{
			m_packageHeader.parseFromBuffer( buff );
			if ( m_packageHeader.m_dwSize != 999 )
			{
				throw std::runtime_error( "echo msg error" );
			}			
			send( buff );
			recv();
			m_staSizeRecv += buff.getDataSize();
		}
		else
		{
			if ( m_bReadHeader )
			{
				m_lastRecvPackage.parseFromBuffer( buff );
			}
			else
			{
				m_packageHeader.parseFromBuffer( buff );
			}
			m_bReadHeader = !m_bReadHeader;
			recv();
			m_staSizeRecv += buff.getDataSize();
		}
	}

	void XClient::onSend( size_t bytesTransferred )
	{
		m_staSizeSend += bytesTransferred;
	}

	void XClient::onResolve()
	{
	}

	void XClient::onClose( size_t id )
	{
		m_bIsConnected = false;
	}
	
	void XClient::onLog( const char* pLog )
	{
		std::string log = outputString( "[%d]%s", m_id, pLog );
		onLogHandler( log.c_str() );
	}
	
	void XClient::onConnTimeoutCallback( const boost::system::error_code& err )
	{
		if ( !m_bIsConnected )
		{
/*
			if ( err )
			{
				onLog( err.message().c_str() );
			}
			else
*/
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

		XAsioPackageHeader header;
		header.m_dwType = 1;
		header.m_dwSize = 999;
		header.m_dwToken = getSendSize();		
		
		recv();

		XAsioBuffer buff;
		buff.clone( &header, sizeof(header) );
		send( buff );
	}

	void XClient::sendTestPackage()
	{
		if ( m_bIsConnected )
		{
			int times = rand() % 1 + 1;
			for ( int i = 0; i < times; i++ )
			{
				XAsioPackage p;
				p.i = 10;
				sprintf_s( p.info, sizeof(p.info), "from client %d", getId() );

				XAsioPackageHeader header;
				header.m_dwSize = sizeof(p);

				XAsioBuffer buff1;
				buff1.copy( &header, sizeof(header) );
				send( buff1 );

				XAsioBuffer buff2;
				buff2.copy( &p, sizeof(p) );
				send( buff2 );
			}
		}
	}

	void XClient::sendThread()
	{
		try
		{
			while( 1 )
			{
				boost::this_thread::interruption_point();

				sendTestPackage();
				
				int millseconds = rand() % 3000 + 2000;
				this_thread::sleep( get_system_time() + posix_time::milliseconds( millseconds ) );
			}
		}
		catch(boost::thread_interrupted &)
		{
		}
	}
}