#include "../../include/asio/XAsioServer.h"
#include "../../include/asio/XAsioBase.h"

namespace XASIO
{
#define MAX_SERVICE_THREAD_NUM		4

#define MAX_ACCEPT_THREAD_NUM		1

	//---------------------------
	// 服务会话
	std::function<void( const char* )>	XServerSession::m_sfuncLogHandler = nullptr;
	size_t XServerSession::m_staSizeRecv = 0;
	size_t XServerSession::m_staSizeSend = 0;

	ServerSessionPtr XServerSession::create( TcpSessionPtr ptr ) { return ServerSessionPtr( new XServerSession( ptr ) )->shared_from_this(); }

	void XServerSession::setLog( std::function<void( const char* )> handler ) { m_sfuncLogHandler = handler; }	
	void XServerSession::disableLog() { m_sfuncLogHandler = nullptr; }
	void XServerSession::onLogHandler( const char* pLog ) { if ( m_sfuncLogHandler != nullptr ) { m_sfuncLogHandler( pLog ); } }
	
	size_t XServerSession::getRecvSize() { return m_staSizeRecv; }
	size_t XServerSession::getSendSize() { return m_staSizeSend; }
	
	XServerSession::XServerSession() : m_tcpSession( nullptr ), m_bIsStarted( false ), m_bReadHeader( false ) {}
	XServerSession::XServerSession( TcpSessionPtr ptr ) : m_tcpSession( ptr ), m_bIsStarted( false ), m_bReadHeader( false ) {}

	bool XServerSession::isStarted() { return m_bIsStarted; }

	bool XServerSession::isOpen() { return m_tcpSession && m_tcpSession->isOpen(); }

	void XServerSession::init( TcpSessionPtr ptr /* = nullptr */ )
	{
		if ( ptr )
		{
			m_tcpSession = ptr->shared_from_this();
		}
		m_tcpSession->setLogHandler( &XServerSession::onLog, this );
		m_tcpSession->setWriteHandler( &XServerSession::onWrite, this );
		m_tcpSession->setReadHandler( &XServerSession::onRecv, this );

		recv();
	}

	void XServerSession::release()
	{
		if ( m_tcpSession )
		{
			m_tcpSession->release();
		}
	}

	void XServerSession::send( XAsioBuffer& buff )
	{
		if ( m_tcpSession && m_tcpSession->isOpen() )
		{
			m_tcpSession->write( buff );
		}
	}

	void XServerSession::close()
	{
		if ( m_tcpSession )
		{
			m_tcpSession->close();
		}
		if ( m_sendThread )
		{
			m_sendThread->interrupt();
		}
	}
	
	void XServerSession::recv()
	{
		if ( !isOpen() )
		{
			return;
		}
		if ( m_bReadHeader && m_packageHeader.m_dwSize > 0 )
		{
			if ( m_packageHeader.m_dwSize != sizeof(XAsioPackage) )
			{
				onLog( outputString( "FATAL ERROR %d", m_packageHeader.m_dwSize ) );
			}
			m_tcpSession->read( m_packageHeader.m_dwSize );
		}
		else
		{
			m_bReadHeader = false;
			m_tcpSession->read( XAsioPackageHeader::getHeaderSize() );			
		}		
	}

	void XServerSession::onRecv( XAsioBuffer& buff )
	{
		if ( m_bReadHeader )
		{
			m_lastPackage.parseFromBuffer( buff );
		}
		else
		{
			m_packageHeader.parseFromBuffer( buff );
		}
		if ( !m_bReadHeader && m_packageHeader.m_dwType == 1 )
		{
			if ( m_packageHeader.m_dwSize != 999 )
			{
				throw std::runtime_error( "echo msg error" );
			}			
			send( buff );
			recv();
		}
		else
		{
			m_bReadHeader = !m_bReadHeader;
			recv();	
		}		
		m_staSizeRecv += buff.getDataSize();
	}

	void XServerSession::onWrite( size_t bytesTransferred )
	{
		m_staSizeSend += bytesTransferred;
	}

	void XServerSession::onClose()
	{
		m_tcpSession->close();
	}

	void XServerSession::onLog( const char* pLog )
	{
		onLogHandler( outputString( "[%d]%s", m_tcpSession ? m_tcpSession->getSessionId() : -1, pLog ) );
	}

	void XServerSession::sendTestPackage()
	{
		XAsioPackage p;
		p.i = m_tcpSession->getSessionId();
		sprintf_s( p.info, sizeof(p.info), "from server" );

		XAsioPackageHeader header;
		header.m_dwFlag = m_tcpSession->getSendSize();
		header.m_dwSize = sizeof(p);

		XAsioBuffer buff1;
		buff1.copy( &header, sizeof(header) );
		send( buff1 );

		XAsioBuffer buff2;
		buff2.copy( &p, header.m_dwSize );
		send( buff2 );
	}
	void XServerSession::sendThread()
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
	void XServerSession::testSend()
	{
		if ( m_sendThread == nullptr )
		{
			m_sendThread = boost::shared_ptr<boost::thread>( new boost::thread( boost::bind( &XServerSession::sendThread, this ) ) );
		}
	}

	//---------------------------
	// 服务

	XServer::XServer( XAsioService& service ) 
		: XAsioPool( service.getIOService() ), m_service( service ),
		m_iAllocateId( 1 ), m_bIsStarted( false ), m_iPort( DEFAULT_XASIO_PORT ),
		 m_iAcceptThreadNum( MAX_ACCEPT_THREAD_NUM ),
		m_funcLogHandler( nullptr ), m_ptrTCPServer( nullptr )
	{
		//allocateObject( 1 );
	}

	XServer::~XServer()
	{
		m_funcLogHandler = nullptr;
	}

	TcpServerPtr XServer::getService() { return m_ptrTCPServer; }
	
	ServerSessionPtr XServer::getSession( unsigned int id )
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		return it != std::end( m_mapSession ) ? it->second : nullptr;
	}

	void XServer::closeSession( unsigned int id )
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		if ( it != std::end( m_mapSession ) )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->release();
			ptr->close();
			m_mapSession.erase( it );
		}
	}
	
	void XServer::setAddress( int port ) { m_iPort = port; }

	void XServer::setAcceptThreadNum( int threadNum ) { m_iAcceptThreadNum = threadNum; }

	unsigned int XServer::queryValidId() { return m_iAllocateId++; }
	
	void XServer::startServer()
	{
		if ( m_bIsStarted )
		{
			return;
		}
		m_bIsStarted = true;

		if ( m_ptrTCPServer == nullptr )
		{
			m_ptrTCPServer = XAsioTCPServer::create( m_service );
		}
		m_ptrTCPServer->setAcceptHandler( &XServer::onAccept, this );
		m_ptrTCPServer->setCancelHandler( &XServer::onCancel, this );
		m_ptrTCPServer->setLogHandler( &XServer::onLog, this );		
		m_ptrTCPServer->startAccept( m_iAcceptThreadNum, m_iPort );

		XServerSession::setLog( std::bind( &XServer::onLog, this, std::placeholders::_1 ) );

		onLog( "start server" );
	}

	void XServer::stopServer()
	{
		if ( !m_bIsStarted )
		{
			return;
		}
		mutex::scoped_lock lock( m_mutex );
		
		XServerSession::disableLog();

		m_bIsStarted = false;		
		m_ptrTCPServer->stopAccept();
		
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->close();
		}
		
		onLog( "stop server" );
	}

	void XServer::sendTo( int id, XAsioBuffer& buff )
	{
		mutex::scoped_lock lock( m_mutex );
		ServerSessionPtr& ptr = getSession( id );
		if ( ptr )
		{
			ptr->send( buff );
		}
	}
	void XServer::sendToAll( XAsioBuffer& buff )
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->send( buff );
		}
	}
	void XServer::sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v )
	{
		mutex::scoped_lock lock( m_mutex );
		int length = v.size();
		for ( int inx = 0; inx < length; inx++ )
		{
			ServerSessionPtr& ptr = getSession( v[inx] );
			ptr->send( buff );
		}
	}

	size_t XServer::getClientCount()
	{
		mutex::scoped_lock lock( m_mutex );
		return m_mapSession.size();
	}

	void XServer::testSend()
	{
		mutex::scoped_lock lock( m_mutex );
		int count = 0;
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->testSend();
			count++;
		}
	}

	void XServer::testSendDirectly()
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->sendTestPackage();
		}
	}

	ServerSessionPtr XServer::createSession()
	{
		//return queryObject();
		return ServerSessionPtr( new XServerSession );
	}
	
	void XServer::onAccept( TcpSessionPtr session )
	{
		mutex::scoped_lock lock( m_mutex );
		
		session->setSessionId( queryValidId() );		
		session->setCloseHandler( &XServer::onSessionClose, this );

		ServerSessionPtr ptr = createSession();
		ptr->init( session );
		m_mapSession.insert( std::make_pair( session->getSessionId(), ptr ) );

		onLog( outputString( "accept [ connect:%d pool:%d temp:%d ]", m_mapSession.size(), getSize(), getClosedSize() ) );
	}

	void XServer::onCancel()
	{
		//onLog( "cancel" );		//will trigger locked
	}
	
	void XServer::onSessionClose( size_t id )
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		if ( it != std::end( m_mapSession ) )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->release();
			ptr->close();
			//releaseObject( it->second );
			m_mapSession.erase( it );
		}
		onLog( outputString( "session %d close [ pool:%d temp:%d ]", id, getSize(), getClosedSize() ) );
	}

	void XServer::onLog( const char* pLog )
	{
		ON_CALLBACK_PARAM( m_funcLogHandler, pLog );
	}
}