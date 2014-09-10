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
#define DEFAULT_SESSION_TIMEROUT_MICROS		10000000
	//600000000

	//---------------------------
	// 服务会话
	XServerSessionPtr XServerSession::create( TcpSessionPtr ptr ) { return XServerSessionPtr( new XServerSession( ptr ) )->shared_from_this(); }

	XServerSession::XServerSession() : 
		m_ptrSession( nullptr ), m_bIsStarted( false ),
		m_funcRecvHandler( nullptr ), m_funcLogHandler( nullptr )
	{
	}

	XServerSession::XServerSession( TcpSessionPtr ptr ) : 
		m_ptrSession( ptr ), m_bIsStarted( false ),
		m_funcRecvHandler( nullptr ), m_funcLogHandler( nullptr )
	{
	}

	XServerSession::~XServerSession()
	{
		m_funcLogHandler	= nullptr;
		m_funcRecvHandler	= nullptr;
	}

	bool XServerSession::isStarted()
	{
		return m_bIsStarted;
	}

	bool XServerSession::isOpen()
	{
		return m_ptrSession && m_ptrSession->isOpen();
	}

	unsigned int XServerSession::getSessionId()
	{
		return m_ptrSession ? m_ptrSession->getSessionId() : 0;
	}

	void XServerSession::init( TcpSessionPtr ptr /* = nullptr */ )
	{
		if ( ptr )
		{
			m_ptrSession = ptr->shared_from_this();
		}
		m_ptrSession->setLogHandler( std::bind( &XServerSession::onLog, this, std::placeholders::_1 ) );
		m_ptrSession->setWriteHandler( std::bind( &XServerSession::onWrite, this, std::placeholders::_1 ) );
		m_ptrSession->setReadHandler( std::bind( &XServerSession::onRecv, this, std::placeholders::_1 ) );

		recv();

		m_lastTickerTime = XTicker::getTickCounter();
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

	bool XServerSession::isTimeout()
	{
		return XTicker::getLastTickCounter() - m_lastTickerTime > DEFAULT_SESSION_TIMEROUT_MICROS;
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
		m_lastTickerTime = XTicker::getTickCounter();
		XAsioStatServerAgent::getMutableInstance()->recv( buff.getDataSize() );
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

	void XServerSession::onWrite( size_t bytesTransferred )
	{
		XAsioStatServerAgent::getMutableInstance()->send( bytesTransferred );
	}

	void XServerSession::onClose()
	{
		m_ptrSession->close();
	}

	void XServerSession::setLogHandler( std::function<void( const char* )> handler )
	{
		m_funcLogHandler = handler;
	}
	void XServerSession::setRecvHandler( std::function<void( XServerSession*, XAsioRecvPacket& )> handler )
	{
		m_funcRecvHandler = handler;
	}

	void XServerSession::onLog( const char* pLog )
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
		m_funcLogHandler = nullptr;
		stopServer();
	}

	TcpServerPtr XServer::getService() { return m_ptrTCPServer; }

	bool XServer::isStarted() { return m_bIsStarted; }

	void XServer::setLogHandler( std::function<void( const char* )> handler )
	{
		m_funcLogHandler = handler;
	}
	void XServer::setConnectHandler( std::function<void( XServerSession* )> handler )
	{
		m_funcAcceptHandler = handler;
	}
	
	XServerSession* XServer::getSession( unsigned int id )
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
			XServerSessionPtr& ptr = it->second;
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
			XServerSessionPtr& ptr = it->second;
			ptr->close();
		}
		
		onLog( "stop server" );
	}

	void XServer::sendTo( int id, XAsioBuffer& buff )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		XServerSession* ptr = getSession( id );
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
			XServerSession* ptr = it->second.get();
			ptr->send( buff );
		}
	}
	void XServer::sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		int length = v.size();
		for ( int inx = 0; inx < length; inx++ )
		{
			XServerSession* ptr = getSession( v[inx] );
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
			XServerSession* ptr = it->second.get();
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
	
	XServerSession* XServer::createSession()
	{
		//return queryObject();
		return new XServerSession;
	}
	
	void XServer::onAccept( TcpSessionPtr session )
	{
		mutex::scoped_lock lock( m_sessionMutex );
		
		session->setSessionId( queryValidId() );		
		session->setCloseHandler( std::bind( &XServer::onSessionClose, this, std::placeholders::_1 ) );
				
		XServerSessionPtr ptr = XServerSessionPtr( createSession() );
		ptr->init( session );
		ptr->setLogHandler( std::bind( &XServer::onLog, this, std::placeholders::_1 ) );
		ptr->setRecvHandler( std::bind( &XServer::onSessionRecv, this, std::placeholders::_1, std::placeholders::_2 ) );
		m_mapSession.insert( std::make_pair( session->getSessionId(), ptr ) );

		XAsioStatServerAgent::getMutableInstance()->connect();

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
			XServerSessionPtr& ptr = it->second;
			ptr->release();
			ptr->close();
			//releaseObject( it->second );
			m_mapSession.erase( it );
		}
		XAsioStatServerAgent::getMutableInstance()->disconnect();
		//onLog( outputString( "session %d close [ pool:%d temp:%d ]", id, getSize(), getClosedSize() ) );
	}

	void XServer::onSessionRecv( XServerSession* pSession, XAsioRecvPacket& packet )
	{
		mutex::scoped_lock lock( m_packetMutex );
		packet.detach();
		packet.setFromId( pSession->getSessionId() );
		m_listRecvPackets.push_back( packet );
		lock.unlock();
		ON_CALLBACK_PARAM( m_funcLogHandler, "recv packet" );
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