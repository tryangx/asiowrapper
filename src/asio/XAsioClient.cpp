#include "../../include/asio/XAsioClient.h"
#include "../../include/asio/XAsioHelper.h"

namespace XASIO
{
	size_t XClient::m_sizeSend = 0;
	size_t XClient::m_sizeRecv = 0;

	std::function<void( std::string )>	XClient::m_sfuncLogHandler = nullptr;

	void XClient::setLog( std::function<void( std::string& )> handler ) { m_sfuncLogHandler = handler; }	
	void XClient::disableLog() { m_sfuncLogHandler = nullptr; }
	void XClient::onLogHandler( std::string& err ) { if ( m_sfuncLogHandler != nullptr ) { m_sfuncLogHandler( err ); } }

	size_t XClient::getSendSize() { return m_sizeSend; }
	size_t XClient::getRecvSize() { return m_sizeRecv; }

	XClient::XClient( XAsioService& io ) : m_service( io ), m_iPort( DEFAULT_XASIO_PORT ),
		m_bInit( false ), m_bIsConnected( false ), m_id( 0 ), m_bReadHeader( false ), m_connectTimer( io ),
		m_ptrTCPClient( nullptr ), m_ptrSession( nullptr )
	{
	}

	XClient::~XClient()
	{
		m_bInit = false;
		m_bIsConnected = false;
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

	void XClient::setAddress( std::string host, int port )
	{
		if ( m_bIsConnected )
		{
			onLogInfo( "Client already connect to server!" );
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
			onLogInfo( "Client isn't initailized" );
			return;
		}
		onLogInfo( "connect" );
		m_ptrTCPClient->setConnectHandler( &XClient::onConnect, this );
		m_ptrTCPClient->setResolveHandler( &XClient::onResolve, this );
		m_ptrTCPClient->setLogHandler( &XClient::onLog, this );
		m_ptrTCPClient->connect( m_sHost, m_iPort );
		
		//m_connectTimer.expires_from_now( boost::posix_time::seconds( 10 ) );
		//m_connectTimer.async_wait( boost::bind( &XClient::disconnect, this ) );

		m_bReadHeader = false;
	}

	void XClient::disconnect()
	{
		if ( m_ptrSession )
		{	
			m_ptrSession->close();
		}
		m_sendThread.interrupt();
		
		onLogInfo( "disconnect" );
	}

	void XClient::testSend()
	{
		m_sendThread = boost::thread( boost::bind( &XClient::sendThread, this ) );
	}

	void XClient::send()
	{
		//预留作为定时发送的扩展
	}

	void XClient::send( std::string content )
	{
		if ( !m_bInit || !m_bIsConnected )
		{
			return;
		}
		if ( m_ptrSession && m_ptrSession->getSocket()->is_open() )
		{
			XAsioBuffer buff = stringToBuffer( content );
			onLogInfo( outputString( "send %d", buff.getDataSize() ) );
			m_ptrSession->write( buff );
		}
	}
	
	void XClient::send( XAsioBuffer& buff )
	{
		if ( !m_bInit || !m_bIsConnected )
		{
			return;
		}
		if ( m_ptrSession && m_ptrSession->getSocket()->is_open() )
		{
			//onLogInfo( outputString( "send %d", buff.getDataSize() ) );
			m_ptrSession->write( buff );
		}
	}

	void XClient::recv()
	{
		if ( m_bReadHeader && m_packageHeader.m_dwSize > 0 )
		{
			m_ptrSession->read( m_packageHeader.m_dwSize );
		}
		else
		{
			m_ptrSession->read( XAsioPackageHeader::getHeaderSize() );
		}		
	}

	void XClient::onConnect( TcpSessionPtr session )
	{
		if ( !m_bInit )
		{
			return;
		}
		m_bIsConnected = true;
		
		m_ptrSession = session;
		m_ptrSession->setReadCompleteHandler( &XClient::onRecvComplete, this );
		m_ptrSession->setReadHandler( &XClient::onRecv, this );
		m_ptrSession->setWriteHandler( &XClient::onSend, this );
		m_ptrSession->setCloseHandler( &XClient::onClose, this );
		m_ptrSession->setLogHandler( &XClient::onLog, this );

		recv();
		testSend();
		onLogInfo( "Client is connect to server!" );
	}

	void XClient::onRecv( XAsioBuffer& buff )
	{
		std::string log = "recv ";
		if ( m_bReadHeader )
		{
			XAsioPackage pack;
			pack.parseFromBuffer( buff );
			log += pack.info;
		}
		else
		{
			m_packageHeader.parseFromBuffer( buff );
			log += "header";
		}
		//onLog( log );
		m_bReadHeader = !m_bReadHeader;
		recv();

		m_sizeRecv += buff.getDataSize();
	}

	void XClient::onRecvComplete()
	{
	}

	void XClient::onSend( size_t bytesTransferred )
	{
		m_sizeSend += bytesTransferred;
		//onLogInfo( outputString( "%d sended", bytesTransferred ) );
	}

	void XClient::onResolve()
	{
		onLogInfo( "resolve" );
	}

	void XClient::onClose( size_t id )
	{
		onLogInfo( "close" );
	}

	void XClient::onLogInfo( const char* pInfo ) { onLog( std::string( pInfo ) ); }

	void XClient::onLog( std::string& err )
	{
		std::string log = outputString( "[%d]%s", m_id, err.c_str() );
		onLogHandler( log );
	}
	
	void XClient::sendThread()
	{
		try
		{
			while( 1 )
			{
				boost::this_thread::interruption_point();

				XAsioPackage p;
				p.i = 10;
				sprintf_s( p.info, sizeof(p.info), "from client %d", getId() );

				XAsioPackageHeader header;
				header.m_dwSize = sizeof(p);

				XAsioBuffer buff;
				buff.copyFrom( &header, sizeof(header) );
				send( buff );
				buff.copyFrom( &p, sizeof(p) );
				send( buff );

				int millseconds = rand() % 3000 + 2000;
				this_thread::sleep( get_system_time() + posix_time::milliseconds( millseconds ) );
			}
		}
		catch(boost::thread_interrupted &)
		{
		}
	}
}