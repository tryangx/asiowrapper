#include "../include/XAsioServer.h"

namespace XASIO
{
#define MAX_SERVICE_THREAD_NUM		4

#define MAX_ACCEPT_THREAD_NUM		2

	//---------------------------
	// 服务会话
	std::function<void( std::string )>	XServerSession::m_sfuncLogHandler = nullptr;
	size_t XServerSession::m_sizeRecv = 0;

	ServerSessionPtr XServerSession::create( TcpSessionPtr ptr )
	{
		return ServerSessionPtr( new XServerSession( ptr ) )->shared_from_this();
	}

	void XServerSession::setLog( std::function<void( std::string& )>& handler ) { m_sfuncLogHandler = handler; }	
	void XServerSession::disableLog() { m_sfuncLogHandler = nullptr; }
	void XServerSession::onLogHandler( std::string& err ) { if ( m_sfuncLogHandler != nullptr ) { m_sfuncLogHandler( err ); } }
	size_t XServerSession::getRecvSize() { return m_sizeRecv; }
	
	XServerSession::XServerSession() : m_tcpSession( nullptr ), m_bIsStarted( false ), m_bReadHeader( false ) {}
	XServerSession::XServerSession( TcpSessionPtr ptr ) : m_tcpSession( ptr ), m_bIsStarted( false ), m_bReadHeader( false ) {}

	bool XServerSession::isStarted() { return m_bIsStarted; }

	void XServerSession::init( TcpSessionPtr ptr /* = nullptr */ )
	{
		if ( ptr )
		{
			m_tcpSession = ptr->shared_from_this();
		}
		m_tcpSession->setLogHandler( &XServerSession::onLog, this );
		m_tcpSession->setWriteHandler( &XServerSession::onWrite, this );
		m_tcpSession->setReadHandler( &XServerSession::onRecv, this );
		m_tcpSession->setReadCompleteHandler( &XServerSession::onRecvComplete, this );

		recv();
		testSend();
	}

	void XServerSession::send( XAsioBuffer& buff )
	{
		if ( m_tcpSession )
		{
			m_tcpSession->write( buff );
			//onLogInfo( "send" );
		}		
	}

	void XServerSession::close()
	{
		if ( m_tcpSession )
		{
			m_tcpSession->close();
		}
		m_sendThread.interrupt();
	}

	void XServerSession::sendThread()
	{
		return;
		try
		{
			while( 1 )
			{
				boost::this_thread::interruption_point();

				XAsioPackage p;
				p.i = 10;
				sprintf_s( p.info, sizeof(p.info), "from server" );

				XAsioPackageHeader header;
				header.m_dwPackageSize = sizeof(p);

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
	void XServerSession::testSend()
	{
		boost::thread( boost::bind( &XServerSession::sendThread, this ) );
	}

	void XServerSession::recv()
	{
		if ( m_bReadHeader && m_packageHeader.m_dwPackageSize > 0 )
		{
			m_tcpSession->read( m_packageHeader.m_dwPackageSize );
		}
		else
		{
			m_bReadHeader = false;
			m_tcpSession->read( XAsioPackageHeader::getSize() );
		}		
	}

	void XServerSession::onRecv( XAsioBuffer buff )
	{
		std::string log = "recv ";
		if ( m_bReadHeader )
		{
			XAsioPackage p;
			p.parseFromBuffer( buff );			
			log += p.info;			
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

	void XServerSession::onRecvComplete()
	{
		onLogInfo( "complete" );
	}

	void XServerSession::onWrite( size_t bytesTransferred )
	{
		//onLogInfo( "write" );
	}

	void XServerSession::onClose()
	{
		m_tcpSession->close();
	}

	void XServerSession::onLog( std::string& err )
	{
		onLogHandler( err );
	}

	void XServerSession::onLogInfo( const char* pInfo )
	{
		onLog( std::string( pInfo ) );
	}

	//---------------------------
	// 服务
	XServer::XServer( XAsioService& service ) 
		: XAsioPool( service ), m_service( service ),
		m_iAllocateId( 1 ), m_bIsStarted( false ), m_iPort( DEFAULT_XASIO_PORT ),
		 m_iAcceptThreadNum( MAX_ACCEPT_THREAD_NUM ),
		m_funcLogHandler( nullptr ), m_ptrTCPServer( nullptr )
	{
		allocateObject( 1 );
	}

	XServer::~XServer()
	{
		m_funcLogHandler = nullptr;
	}

	TcpServerPtr XServer::getService() { return m_ptrTCPServer; }
	
	ServerSessionPtr XServer::getSession( unsigned int id )
	{
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id ); return it != std::end( m_mapSession ) ? it->second : nullptr;
	}
	
	void XServer::setAddress( int port ) { m_iPort = port; }

	void XServer::setAcceptThreadNum( int threadNum ) { m_iAcceptThreadNum = threadNum; }

	unsigned int XServer::queryValidId() { return m_iAllocateId++; }

	void XServer::init() {}
	void XServer::release() {}

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

		std::function<void(std::string&)> handler = std::bind( &XServer::onLog, this, std::placeholders::_1 );
		XServerSession::setLog( handler );

		onLogInfo( "start server" );
	}

	void XServer::stopServer()
	{
		if ( !m_bIsStarted )
		{
			return;
		}
		mutex::scoped_lock lock( m_mutex );

		m_bIsStarted = false;
		
		m_ptrTCPServer->stopAccept();
		
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->close();
		}

		XServerSession::disableLog();
		
		lock.unlock();
		onLogInfo( "stop server" );
	}

	void XServer::sendTo( int id, XAsioBuffer& buff )
	{
		ServerSessionPtr& ptr = getSession( id );
		if ( ptr )
		{
			ptr->send( buff );
		}
	}
	void XServer::sendToAll( XAsioBuffer& buff )
	{
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->send( buff );
		}
	}
	void XServer::sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v )
	{
		int length = v.size();
		for ( int inx = 0; inx < length; inx++ )
		{
			ServerSessionPtr& ptr = getSession( v[inx] );
			ptr->send( buff );
		}
	}

	void XServer::testSend()
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->testSend();
		}
		lock.unlock();
	}
	
	void XServer::onAccept( TcpSessionPtr session )
	{
		mutex::scoped_lock lock( m_mutex );
		//ServerSessionPtr ptr = XServerSession::create( session );

		session->setId( queryValidId() );		
		session->setCloseHandler( &XServer::onSessionClose, this );

		ServerSessionPtr ptr = queryObject();
		ptr->init( session );
		
		m_mapSession.insert( std::make_pair( session->getId(), ptr ) );
		onLogInfo( "accept" );
		onLogInfo( outputString( "connect:%d pool:%d temp:%d", m_mapSession.size(), getSize(), getClosedSize() ) );
	}

	void XServer::onCancel()
	{
		onLogInfo( "cancel" );
	}
	
	void XServer::onSessionClose( size_t id )
	{
		mutex::scoped_lock lock( m_mutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		if ( it != std::end( m_mapSession ) )
		{
			releaseObject( it->second );
			m_mapSession.erase( it );
		}
		lock.unlock();
		onLogInfo( "session close" );
		onLogInfo( outputString( "pool:%d temp:%d", getSize(), getClosedSize() ) );
	}

	void XServer::onLog( std::string& err ) { if ( m_funcLogHandler != nullptr ) m_funcLogHandler( err ); }

	void XServer::onLogInfo( const char* pInfo ) { onLog( std::string( pInfo ) ); }
}