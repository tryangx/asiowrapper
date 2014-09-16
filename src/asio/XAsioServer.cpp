#include "asio/XAsioServer.h"
#include "asio/XAsioBase.h"
#include "util/XStringUtil.h"
#include "util/XTicker.h"

namespace XGAME
{
/**
 * 服务的线程数量
 */
#define DEFAULT_SERVICE_THREAD_NUM			4

/**
 * 侦听线程的数量
 */
#define DEFAULT_ACCEPT_THREAD_NUM			1

/**
 * 会话超时的时间（600秒）
 */
#define DEFAULT_SESSION_TIMEROUT_MICROS		600000000

	//---------------------------
	// 服务会话
	TCPSrvSessionPtr XAsioTCPSrvSession::create( TcpSessionPtr ptr ) { return TCPSrvSessionPtr( new XAsioTCPSrvSession( ptr ) )->shared_from_this(); }

	XAsioTCPSrvSession::XAsioTCPSrvSession() : 
		m_ptrSession( nullptr ), m_bIsStarted( false ),
		m_funcRecvHandler( nullptr ), m_funcLogHandler( nullptr ), m_pStat( NULL )
	{
	}

	XAsioTCPSrvSession::XAsioTCPSrvSession( TcpSessionPtr ptr ) : 
		m_ptrSession( ptr ), m_bIsStarted( false ),
		m_funcRecvHandler( nullptr ), m_funcLogHandler( nullptr ), m_pStat( NULL )
	{
	}

	XAsioTCPSrvSession::~XAsioTCPSrvSession()
	{
		m_funcLogHandler	= nullptr;
		m_funcRecvHandler	= nullptr;
	}

	bool XAsioTCPSrvSession::isStarted()
	{
		return m_bIsStarted;
	}

	bool XAsioTCPSrvSession::isOpen()
	{
		return m_ptrSession && m_ptrSession->isOpen();
	}

	unsigned int XAsioTCPSrvSession::getSessionId()
	{
		return m_ptrSession ? m_ptrSession->getSessionId() : 0;
	}

	void XAsioTCPSrvSession::setStat( XAsioStat* pStat )
	{
		m_pStat = pStat;
	}

	void XAsioTCPSrvSession::init( TcpSessionPtr ptr /* = nullptr */ )
	{
		if ( ptr )
		{
			m_ptrSession = ptr->shared_from_this();
		}
		m_ptrSession->setLogHandler( std::bind( &XAsioTCPSrvSession::onLog, this, std::placeholders::_1 ) );
		m_ptrSession->setWriteHandler( std::bind( &XAsioTCPSrvSession::onWrite, this, std::placeholders::_1 ) );
		m_ptrSession->setReadHandler( std::bind( &XAsioTCPSrvSession::onRecv, this, std::placeholders::_1 ) );

		recv();

		m_lastTickerTime = XTicker::getTickCounter();
	}

	void XAsioTCPSrvSession::release()
	{
		if ( m_ptrSession )
		{
			m_ptrSession->release();
		}
	}

	void XAsioTCPSrvSession::send( XAsioBuffer& buff )
	{
		if ( m_ptrSession && m_ptrSession->isOpen() )
		{
			m_ptrSession->write( buff );
		}
	}

	bool XAsioTCPSrvSession::isTimeout()
	{
		return XTicker::getLastTickCounter() - m_lastTickerTime > DEFAULT_SESSION_TIMEROUT_MICROS;
	}

	void XAsioTCPSrvSession::close()
	{
		if ( m_ptrSession )
		{
			m_ptrSession->close();
		}
	}
	
	void XAsioTCPSrvSession::recv()
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

	void XAsioTCPSrvSession::onRecv( XAsioBuffer& buff )
	{
		m_lastTickerTime = XTicker::getTickCounter();
		if ( m_pStat )
		{
			m_pStat->recv( buff.getDataSize() );
		}
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

	void XAsioTCPSrvSession::onWrite( size_t bytesTransferred )
	{
		if ( m_pStat )
		{
			m_pStat->send( bytesTransferred );
		}
	}

	void XAsioTCPSrvSession::onClose()
	{
		m_ptrSession->close();
	}

	void XAsioTCPSrvSession::setLogHandler( std::function<void( const char* )> handler )
	{
		m_funcLogHandler = handler;
	}
	void XAsioTCPSrvSession::setRecvHandler( std::function<void( XAsioTCPSrvSession*, XAsioRecvPacket& )> handler )
	{
		m_funcRecvHandler = handler;
	}

	void XAsioTCPSrvSession::onLog( const char* pLog )
	{
		std::string s = pLog;
		ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "[%d]%s", m_ptrSession ? m_ptrSession->getSessionId() : -1, s.c_str() ) );
	}

	//---------------------------
	// 服务

	XServer::XServer( XAsioService& service ) 
		: m_service( service ), //XAsioPool( service.getIOService() ),
		m_iAllocateId( CLIENT_START_ID ), m_bIsStarted( false ), m_iPort( 6580 ),
		m_iAcceptThreadNum( DEFAULT_ACCEPT_THREAD_NUM ),
		m_funcLogHandler( nullptr ), m_ptrTCPServer( nullptr ), m_timerUpdateTimer( service.getIOService() )
	{
		//allocateObject( 1 );
	}

	XServer::~XServer()
	{
		m_funcLogHandler	=	nullptr;
		m_funcAcceptHandler	=	nullptr;
		stopServer();
	}

	TcpServerPtr XServer::getService() { return m_ptrTCPServer; }

	XAsioStat* XServer::getStat() { return &m_stat; }

	bool XServer::isStarted() { return m_bIsStarted; }

	void XServer::setLogHandler( std::function<void( const char* )> handler )
	{
		m_funcLogHandler = handler;
	}
	void XServer::setConnectHandler( std::function<void( XAsioTCPSrvSession* )> handler )
	{
		m_funcAcceptHandler = handler;
	}
	
	XAsioTCPSrvSession* XServer::getSession( unsigned int id )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		return it != std::end( m_mapSession ) ? it->second.get() : nullptr;
	}

	void XServer::closeSession( unsigned int id )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		MAPSERVERSESSIONPTR::iterator it = m_mapSession.find( id );
		if ( it != std::end( m_mapSession ) )
		{
			TCPSrvSessionPtr& ptr = it->second;
			ptr->release();
			ptr->close();
			m_mapSession.erase( it );
		}
	}
	
	void XServer::setAddress( int port ) { m_iPort = port; }

	void XServer::setAcceptThreadNum( int threadNum ) { m_iAcceptThreadNum = threadNum; }

	unsigned int XServer::queryValidId()
	{
		if ( m_iAllocateId + 1 < CLIENT_START_ID )
		{
			m_iAllocateId = CLIENT_START_ID;
		}
		return m_iAllocateId++;
	}
	
	void XServer::startServer()
	{
		if ( m_bIsStarted )
		{
			return;
		}
		m_bIsStarted = true;
		
		m_ptrTCPServer.reset();
		m_ptrTCPServer = XAsioTCPServer::create( m_service );
		m_ptrTCPServer->setAcceptHandler( std::bind( &XServer::onAccept, this, std::placeholders::_1 ) );
		m_ptrTCPServer->setCancelHandler( std::bind( &XServer::onCancel, this ) );
		m_ptrTCPServer->setLogHandler( std::bind( &XServer::onLog, this, std::placeholders::_1 ) );		
		m_ptrTCPServer->startAccept( m_iAcceptThreadNum, m_iPort );
		
		m_timerUpdateTimer.expires_from_now( posix_time::millisec( 5000 ) );
		m_timerUpdateTimer.async_wait( boost::bind( &XServer::onTimerUpdateCallback, this, boost::asio::placeholders::error ) );

		onLog( "start server" );
	}

	void XServer::stopServer()
	{
		if ( !m_bIsStarted )
		{
			return;
		}
		mutex::scoped_lock lock( m_sessionMutex );
		
		m_bIsStarted = false;
		m_ptrTCPServer->release();
		m_ptrTCPServer->stopAccept();
		
		m_timerUpdateTimer.cancel();
		
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); it++ )
		{
			TCPSrvSessionPtr& ptr = it->second;
			ptr->close();
		}
		
		onLog( "stop server" );
	}

	void XServer::sendTo( int id, XAsioBuffer& buff )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		XAsioTCPSrvSession* ptr = getSession( id );
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
			XAsioTCPSrvSession* ptr = it->second.get();
			ptr->send( buff );
		}
	}
	void XServer::sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		int length = v.size();
		for ( int inx = 0; inx < length; inx++ )
		{
			XAsioTCPSrvSession* ptr = getSession( v[inx] );
			if ( ptr )
			{
				ptr->send( buff );
			}
		}
	}

	void XServer::updateClient()
	{
		mutex::scoped_lock lock( m_sessionMutex );
		XTicker::getTickCounter();
		MAPSERVERSESSIONPTR::iterator it = std::begin( m_mapSession );
		for ( ; it != std::end( m_mapSession ); )
		{
			XAsioTCPSrvSession* ptr = it->second.get();
			if ( ptr->isTimeout() )
			{
				onLog( outputString( "Client [%d] timeout", ptr->getSessionId() ) );
				ptr->release();
				ptr->close();
				it = m_mapSession.erase( it );
				continue;
			}
			it++;		
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
	
	XAsioTCPSrvSession* XServer::createSession()
	{
		//return queryObject();
		return new XAsioTCPSrvSession;
	}
	
	void XServer::onAccept( TcpSessionPtr session )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		
		session->setSessionId( queryValidId() );		
		session->setCloseHandler( std::bind( &XServer::onSessionClose, this, std::placeholders::_1 ) );
				
		TCPSrvSessionPtr ptr = TCPSrvSessionPtr( createSession() );
		ptr->setStat( &m_stat );
		ptr->init( session );
		ptr->setLogHandler( std::bind( &XServer::onLog, this, std::placeholders::_1 ) );
		ptr->setRecvHandler( std::bind( &XServer::onSessionRecv, this, std::placeholders::_1, std::placeholders::_2 ) );
		m_mapSession.insert( std::make_pair( session->getSessionId(), ptr ) );

		m_stat.connect();

		ON_CALLBACK_PARAM( m_funcAcceptHandler, ptr.get() );
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
			TCPSrvSessionPtr& ptr = it->second;
			ptr->release();
			ptr->close();
			//releaseObject( it->second );
			m_mapSession.erase( it );
		}
		m_stat.disconnect();
		//onLog( outputString( "session %d close [ pool:%d temp:%d ]", id, getSize(), getClosedSize() ) );
	}

	void XServer::onSessionRecv( XAsioTCPSrvSession* pSession, XAsioRecvPacket& packet )
	{
		mutex::scoped_lock lock( m_packetMutex );
		packet.detach();
		packet.setFromId( pSession->getSessionId() );
		m_listRecvPackets.push_back( packet );
		lock.unlock();
		ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "recv packet from %d", pSession->getSessionId() ) );
	}

	void XServer::onLog( const char* pLog )
	{
		ON_CALLBACK_PARAM( m_funcLogHandler, pLog );
	}

	void XServer::onTimerUpdateCallback( const boost::system::error_code& err )
	{
		if ( err == nullptr && m_bIsStarted )
		{
			updateClient();
			m_timerUpdateTimer.expires_from_now( posix_time::millisec( 5000 ) );
			m_timerUpdateTimer.async_wait( boost::bind( &XServer::onTimerUpdateCallback, this, boost::asio::placeholders::error ) );
		}
	}
}