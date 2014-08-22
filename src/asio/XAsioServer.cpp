#include "asio/XAsioServer.h"
#include "asio/XAsioBase.h"
#include "util/XStringUtil.h"

namespace XGAME
{
#define MAX_SERVICE_THREAD_NUM		4

#define MAX_ACCEPT_THREAD_NUM		1

	//---------------------------
	// 服务会话
	std::function<void( const char* )>	XServerSession::m_sfuncLogHandler = nullptr;
	std::function<void( XServerSession&, XAsioRecvPacket& )> XServerSession::m_sfuncRecvHandler = nullptr;
	
	ServerSessionPtr XServerSession::create( TcpSessionPtr ptr ) { return ServerSessionPtr( new XServerSession( ptr ) )->shared_from_this(); }

	void XServerSession::setLogHandler( std::function<void( const char* )> handler ) { m_sfuncLogHandler = handler; }	
	void XServerSession::setRecvHandler( std::function<void( XServerSession&, XAsioRecvPacket& )> handler ) { m_sfuncRecvHandler = handler; }
	void XServerSession::onLogHandler( const char* pLog ) { if ( m_sfuncLogHandler != nullptr ) { m_sfuncLogHandler( pLog ); } }
		
	XServerSession::XServerSession() : m_ptrSession( nullptr ), m_bIsStarted( false ) {}
	XServerSession::XServerSession( TcpSessionPtr ptr ) : m_ptrSession( ptr ), m_bIsStarted( false ) {}

	bool XServerSession::isStarted() { return m_bIsStarted; }

	bool XServerSession::isOpen() { return m_ptrSession && m_ptrSession->isOpen(); }

	void XServerSession::init( TcpSessionPtr ptr /* = nullptr */ )
	{
		if ( ptr )
		{
			m_ptrSession = ptr->shared_from_this();
		}
		m_ptrSession->setLogHandler( &XServerSession::onLog, this );
		m_ptrSession->setWriteHandler( &XServerSession::onWrite, this );
		m_ptrSession->setReadHandler( &XServerSession::onRecv, this );

		recv();
	}

	void XServerSession::release()
	{
		if ( m_ptrSession )
		{
			m_ptrSession->release();
		}
	}

	void XServerSession::send( XAsioBuffer& buff )
	{
		if ( m_ptrSession && m_ptrSession->isOpen() )
		{
			m_ptrSession->write( buff );
		}
	}

	void XServerSession::close()
	{
		if ( m_ptrSession )
		{
			m_ptrSession->close();
		}
	}
	
	void XServerSession::recv()
	{
		if ( !isOpen() )
		{
			return;
		}
		if ( m_recvPacket.isEmpty() )
		{
			m_ptrSession->read( XAsioPacketHeader::getHeaderSize() );
		}
		else if ( m_recvPacket.getHeader() )
		{
			m_ptrSession->read( m_recvPacket.getHeader()->m_dwSize );
		}
	}

	void XServerSession::onRecv( XAsioBuffer& buff )
	{
		XAsioStatServerAgent::getMutableInstance()->recv( buff.getDataSize() );
		m_recvPacket.import( buff );
		if ( m_recvPacket.isReady() )
		{
			if ( m_recvPacket.getHeader()->m_cOp == EN_POP_ECHO )
			{
				send( buff );
			}
			ON_CALLBACK_PARAM2( m_sfuncRecvHandler, *this, m_recvPacket );
		}
		recv();
	}

	void XServerSession::onWrite( size_t bytesTransferred )
	{
		XAsioStatServerAgent::getMutableInstance()->send( bytesTransferred );
	}

	void XServerSession::onClose()
	{
		m_ptrSession->close();
	}

	void XServerSession::onLog( const char* pLog )
	{
		std::string s = pLog;
		onLogHandler( outputString( "[%d]%s", m_ptrSession ? m_ptrSession->getSessionId() : -1, s.c_str() ) );
	}

	//---------------------------
	// 服务

	XServer::XServer( XAsioService& service ) 
		: m_service( service ),
		//XAsioPool( service.getIOService() ),
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
		mutex::scoped_lock lock( m_sessionMutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		return it != std::end( m_mapSession ) ? it->second : nullptr;
	}

	void XServer::closeSession( unsigned int id )
	{
		mutex::scoped_lock lock( m_sessionMutex );
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

		XServerSession::setLogHandler( std::bind( &XServer::onLog, this, std::placeholders::_1 ) );
		XServerSession::setRecvHandler( std::bind( &XServer::onSessionRecv, this, std::placeholders::_1, std::placeholders::_2 ) );

		onLog( "start server" );
	}

	void XServer::stopServer()
	{
		if ( !m_bIsStarted )
		{
			return;
		}
		mutex::scoped_lock lock( m_sessionMutex );
		
		XServerSession::setLogHandler( nullptr );
		XServerSession::setRecvHandler( nullptr );

		m_bIsStarted = false;		
		m_ptrTCPServer->stopAccept();
		m_ptrTCPServer.reset();
		
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
		mutex::scoped_lock lock( m_sessionMutex );
		ServerSessionPtr& ptr = getSession( id );
		if ( ptr )
		{
			ptr->send( buff );
		}
	}
	void XServer::sendToAll( XAsioBuffer& buff )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->send( buff );
		}
	}
	void XServer::sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		int length = v.size();
		for ( int inx = 0; inx < length; inx++ )
		{
			ServerSessionPtr& ptr = getSession( v[inx] );
			ptr->send( buff );
		}
	}

	size_t XServer::getClientCount()
	{
		mutex::scoped_lock lock( m_sessionMutex );
		return m_mapSession.size();
	}

	bool XServer::queryRecvPacket( XAsioRecvPacket& packet )
	{
		mutex::scoped_lock lock( m_packetMutex );
		if ( !m_listRecvPackets.empty() )
		{
			XAsioRecvPacket& temp = m_listRecvPackets.front();
			packet.import( temp );
			packet.attach();
			m_listRecvPackets.pop_front();
			return true;
		}
		return false;
	}
	
	ServerSessionPtr XServer::createSession()
	{
		//return queryObject();
		return ServerSessionPtr( new XServerSession );
	}
	
	void XServer::onAccept( TcpSessionPtr session )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		
		session->setSessionId( queryValidId() );		
		session->setCloseHandler( &XServer::onSessionClose, this );

		ServerSessionPtr ptr = createSession();
		ptr->init( session );
		m_mapSession.insert( std::make_pair( session->getSessionId(), ptr ) );

		XAsioStatServerAgent::getMutableInstance()->connect();
		//onLog( outputString( "accept [ connect:%d pool:%d temp:%d ]", m_mapSession.size(), getSize(), getClosedSize() ) );
	}

	void XServer::onCancel()
	{
		//onLog( "cancel" );		//will trigger locked
	}
	
	void XServer::onSessionClose( size_t id )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		if ( it != std::end( m_mapSession ) )
		{
			ServerSessionPtr& ptr = it->second;
			ptr->release();
			ptr->close();
			//releaseObject( it->second );
			m_mapSession.erase( it );
		}
		XAsioStatServerAgent::getMutableInstance()->disconnect();
		//onLog( outputString( "session %d close [ pool:%d temp:%d ]", id, getSize(), getClosedSize() ) );
	}

	void XServer::onSessionRecv( XServerSession& session, XAsioRecvPacket& packet )
	{
		mutex::scoped_lock lock( m_packetMutex );
		packet.detach();
		m_listRecvPackets.push_back( packet );
		lock.unlock();
	}

	void XServer::onLog( const char* pLog )
	{
		ON_CALLBACK_PARAM( m_funcLogHandler, pLog );
	}
}