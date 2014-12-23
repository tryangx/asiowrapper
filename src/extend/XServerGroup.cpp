#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{
#define DEFAULT_THREAD		1

	stAppServerEndPoint::stAppServerEndPoint() : _ip( NULL ), _port( 0 ), _type( EN_APPSERVER_COUNT ) {}
	stAppServerEndPoint::stAppServerEndPoint( const char* ip, short port, enAppServerType type ) : _ip( ip ), _port( port ), _type( type ) {}
	
	stAppServerConfig::stAppServerConfig() : _listenPort( 0 )
	{
	}

	void stAppServerConfig::clone( stAppServerConfig& config )
	{
		_listenPort		= config._listenPort;
		_serverType		= config._serverType;
		_sName			= config._sName;
		_mapSrvEndPoint	= config._mapSrvEndPoint;
	}

	void stAppServerConfig::addEndPoint( stAppServerEndPoint& ep )
	{
		_mapSrvEndPoint[ep._type] = ep;
	}

	//----------≤‚ ‘
	//  DB <- WORLD  -> LOG
	//			|
	//        GATE
	//			|
	//		  CLIENT

	void stAppServerConfig::testGateConfig()
	{
		_sName = "[gate]";
		_serverType = EN_APPSERVER_GATE;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
	}
	void stAppServerConfig::testWorldConfig()
	{
		_sName = "[world]";
		_serverType = EN_APPSERVER_WORLD;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_DB, EN_APPSERVER_DB ) );
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_LOG, EN_APPSERVER_LOG ) );
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_GATE, EN_APPSERVER_GATE ) );
	}
	void stAppServerConfig::testDBConfig()
	{
		_sName = "[db]";
		_serverType = EN_APPSERVER_DB;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
	}
	void stAppServerConfig::testLogConfig()
	{
		_sName = "[log]";
		_serverType = EN_APPSERVER_LOG;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
	}

	void stAppServerConfig::testCenterConfig()
	{
		_sName = "[center]";
		_serverType = EN_APPSERVER_CENTER;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_WORLD, EN_APPSERVER_WORLD ) );
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_GATE, EN_APPSERVER_GATE ) );
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_LOG, EN_APPSERVER_LOG ) );
		addEndPoint( stAppServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_DB, EN_APPSERVER_DB ) );
	}

	//--------Connector Server

	XAppConnector::XAppConnector( XAsioServiceController& controller ) : m_controller( controller ), XClient( controller ), m_serverType( EN_APPSERVER_UNKNOW )
	{
	}

	XAppConnector::~XAppConnector()
	{
		if ( m_ptrTCPClient )
		{
			m_ptrTCPClient->setReconnectHandler( nullptr );
		}
		if ( getService() )
		{
			m_controller.removeService( getService() );
		}
	}

	void XAppConnector::setConnectorId( unsigned int id )
	{
		m_iConnectId = id;
	}

	void XAppConnector::setServerType( enAppServerType type, std::string name )
	{
		m_serverType = type;
		m_serverName = name;
	}
	
	void XAppConnector::onConnect( TcpSessionPtr session )
	{
		XClient::onConnect( session );
		
		//register
		XAsioSendPacket packet( 0 );
		packet.getHeader()->setOp( EN_POP_REGISTER );
		packet << int( m_serverType ) << int( m_iConnectId ) << m_serverName;
		send( packet );

		onLog( outputString( "send register server=%d curid=%d", m_serverType, m_iConnectId ) );
	}
	void XAppConnector::onRecv( XAsioBuffer& buff )
	{
		XClient::onRecv( buff );
	}
	void XAppConnector::onReconnectCallback()
	{
		onLog( "reconnect" );
		connect( m_serverEndpoint );
	}

	bool XAppConnector::connect( stAppServerEndPoint& ep )
	{
		if ( isConnected() )
		{
			return false;
		}
		m_serverEndpoint = ep;
		setAddress( m_serverEndpoint._ip, m_serverEndpoint._port );
		if ( XClient::connect() )
		{
			if ( m_ptrTCPClient )
			{
				m_ptrTCPClient->setReconnectHandler( std::bind( &XAppConnector::onReconnectCallback, this ) );
			}
			m_controller.startService( getService(), 1 );
		}
		return true;
	}

	void XAppConnector::send( XAsioBuffer& buff )
	{
		XClient::send( buff );
	}

	void XAppConnector::send( XAsioSendPacket& packet )
	{
		XAsioBuffer buff;
		packet.output( buff );
		XClient::send( buff );
	}

	//--------App server
	std::string	XAppServer::m_sAppServerName[EN_APPSERVER_COUNT] = {
		"gate",
		"center",
		"world",
		"db",
		"log",
	};

	XAppServer::XAppServer() : m_bConfigLoaded( false ), m_funcLogHandler( nullptr )
	{
	}
	XAppServer::~XAppServer()
	{
		m_funcLogHandler = nullptr;

		m_procPacketThread.interrupt();
		
		stopServer();

		if ( m_ptrService && m_ptrServer )
		{
			m_ptrService->removeService( m_ptrServer->getService().get() );
		}
	}

	int	XAppServer::getConnectorCnt()
	{
		int cnt = 0;
		for ( int i = 0; i < EN_APPSERVER_COUNT; i++ )
		{
			if ( m_ptrConnectors[i] )
			{
				cnt++;
			}
		}
		return cnt;
	}

	int	XAppServer::getAppServerCnt()
	{
		int cnt = 0;
		for ( int i = 0; i < EN_APPSERVER_COUNT; i++ )
		{
			if ( m_mapAppSrvSession[i].size() )
			{
				cnt++;
			}
		}
		return cnt;
	}

	int	XAppServer::getClientCnt()
	{
		return m_ptrServer ? m_ptrServer->getClientCount() : 0;
	}

	XServer* XAppServer::getServer()
	{
		return m_ptrServer.get();
	}

	void XAppServer::setIoService( boost::shared_ptr<class XAsioServiceController>& ioService )
	{
		m_ptrService = ioService;
	}

	void XAppServer::queryAddress( std::string& ip, int& port )
	{
		ip = "localhost";
		port = m_serverConfig._listenPort;
	}

	bool XAppServer::loadConfig( stAppServerConfig& config )
	{
		m_bConfigLoaded = true;
		m_serverConfig.clone( config );
		return true;
	}

	bool XAppServer::startServer()
	{
		if ( !m_bConfigLoaded || m_ptrService == nullptr )
		{
			return false;
		}
		if ( m_ptrServer && m_ptrServer->isStarted() )
		{
			return false;
		}
		m_ptrServer.reset();
		m_ptrServer = XServerPtr( new XServer( *m_ptrService.get() ) );
		//m_ptrServer->setLogHandler( std::bind( &XAppServer::onLog, this, std::placeholders::_1 ) );
		m_ptrServer->setConnectHandler( std::bind( &XAppServer::onAccept, this, std::placeholders::_1 ) );
		m_ptrServer->setAddress( m_serverConfig._listenPort );
		m_ptrServer->setAcceptThreadNum( 1 );
		m_ptrServer->startServer();
		m_ptrService->startService( m_ptrServer->getService().get(), DEFAULT_THREAD );
		startProcessPacket();
		onLog( outputString( "port %d", m_serverConfig._listenPort ) );
		return true;
	}

	bool XAppServer::connectAppServer()
	{
		std::unordered_map<int,stAppServerEndPoint>::iterator it = m_serverConfig._mapSrvEndPoint.begin();
		for ( ; it != m_serverConfig._mapSrvEndPoint.end(); it++ )
		{
			stAppServerEndPoint& ep = it->second;
			if ( ep._port == 0 )
			{
				continue;
			}
			XAppConnector* pConn = getConnector( ep._type );
			if ( !pConn )
			{
				m_ptrConnectors[ep._type] = XAppConnectorPtr( new XAppConnector( *m_ptrService.get() ) );
				pConn = getConnector( ep._type );
				pConn->setConnectorId( ep._type );
				pConn->setServerType( m_serverConfig._serverType, m_serverConfig._sName );
				//pConn->setLogHandler( std::bind( &XAppServer::onLog, this, std::placeholders::_1 ) );
				pConn->setRecvHandler( std::bind( &XAppServer::onConnectorRecv, this, std::placeholders::_1, std::placeholders::_2 ) );
			}
			if ( !pConn->isConnected() )
			{
				pConn->connect( ep );
				onLog( outputString( "try conn %s %d", ep._ip, ep._port ) );
			}
		}
		return true;
	}

	bool XAppServer::stopServer()
	{
		std::unordered_map<int,stAppServerEndPoint>::iterator it = m_serverConfig._mapSrvEndPoint.begin();
		for ( ; it != m_serverConfig._mapSrvEndPoint.end(); it++ )
		{
			stAppServerEndPoint& ep = it->second;
			if ( ep._port != 0 )
			{
				XAppConnector* pConn = getConnector( ep._type );
				if (  pConn )
				{
					pConn->disconnect();
				}
			}
		}
		if ( m_ptrServer )
		{
			m_ptrServer->stopServer();
		}
		if ( m_ptrService )
		{
			m_ptrService->stopService( m_ptrServer->getService().get() );
		}
		return true;
	}

	void XAppServer::startProcessPacket()
	{
		if ( !m_ptrServer || !m_ptrServer->isStarted() )
		{
			return;
		}
		m_procPacketThread = thread( boost::bind( &XAppServer::onProcessPacketThread, this ) );
	}

	void XAppServer::onProcessPacketThread()
	{
		XAsioRecvPacket packet;
		while( m_ptrServer->isStarted() )
		{
			boost::this_thread::interruption_point();
			if ( m_ptrServer->queryRecvPacket( packet ) )
			{
				onProcessPacket( packet );
			}
			else
			{
				this_thread::sleep( get_system_time() + posix_time::milliseconds( 20 ) );
			}
		}
	}
	XAppConnector* XAppServer::getConnector( enAppServerType type )
	{
		if ( m_ptrConnectors[type] )
		{
			return m_ptrConnectors[type].get();
		}
		return nullptr;
	}
	XAsioTCPSrvSession*	XAppServer::getAppServer( enAppServerType type )
	{
		MAP_APPSERVER_SESSION& container = m_mapAppSrvSession[type];
		XAsioTCPSrvSession* pSession = container.empty() ? NULL : container.begin()->second;
		return pSession;
	}
	void XAppServer::sendToServer( enAppServerType type, XAsioBuffer& buffer )
	{
		XAppConnector* pConn = getConnector( type );
		if ( pConn )
		{
			onLog( "send connector msg" );
			pConn->send( buffer );
			return;
		}
		XAsioTCPSrvSession* pSession = getAppServer( type );
		if ( pSession )
		{
			onLog( outputString( "send session=%d msg", pSession->getSessionId() ) );
			pSession->send( buffer );
			return;
		}
		onLog( outputString( "Server %s isn't connected!", m_sAppServerName[type].c_str() ) );
	}
	void XAppServer::sendToServer( enAppServerType type, XAsioSendPacket& packet )
	{
		XAsioBuffer buff;
		packet.output( buff );
		sendToServer( type, buff );
	}
	void XAppServer::onConnectorRecv( XClient* pClient, XAsioRecvPacket& packet )
	{
		onProcessPacket( packet );
	}
	void XAppServer::onLog( const char* pLog )
	{
		if ( m_funcLogHandler != nullptr )
		{
			std::string s = m_serverConfig._sName;
			s += pLog;
			m_funcLogHandler( s.c_str() );
		}
	}
	void XAppServer::onAccept( XAsioTCPSrvSession* pSession )
	{
		onLog( outputString( "accept %d", pSession->getSessionId() ) );//m_serverConfig._listenPort ) );
	}

	void XAppServer::onProcessPacket( XAsioRecvPacket& recv )
	{
		try
		{
			unsigned char op = recv.getHeader()->getOp();
			if ( op == EN_POP_CMD )
			{
				onLog( outputString( "recv cmd dest=%d", recv.getHeader()->getDestID() ) );
				onProcessCmdPacket( recv );
			}
			else if ( op == EN_POP_MSG )
			{
				onLog( outputString( "recv msg type=%d", recv.getHeader()->getType() ) );
				onProcessMsgPacket( recv );
			}
			else if ( op == EN_POP_HEARTBEAT )
			{
				onLog( "recv heartbeat" );
				onProcessHeartBeatPacket( recv );
			}
			else if ( op == EN_POP_REGISTER )
			{
				onProcessRegPacket( recv );
			}
			else if ( op == EN_POP_GM )
			{
				onProcessGMPacket( recv );
			}
			else if ( op == EN_POP_ECHO )
			{
				onProcessEchoPacket( recv );
			}
		}
		catch(std::runtime_error&e)
		{
			onLog( e.what() );
		}
		catch(...)
		{
			onLog( "unknow error" );
		}
	}

	void XAppServer::onProcessCmdPacket( XAsioRecvPacket& recv )
	{
		unsigned int destId = recv.getHeader()->getDestID();
		XAsioBuffer buffer;
		recv.exportBuffer( buffer );
		if ( destId == m_serverConfig._serverType )
		{
			onProcessMsgPacket( recv );
			return;
		}
		switch( destId )
		{
		case EN_APPSERVER_GATE:
			sendToServer( EN_APPSERVER_GATE, buffer );
			break;
		case EN_APPSERVER_CENTER:
			sendToServer( EN_APPSERVER_CENTER, buffer );
			break;
		case EN_APPSERVER_WORLD:
			sendToServer( EN_APPSERVER_WORLD, buffer );
			break;
		case EN_APPSERVER_DB:
			sendToServer( EN_APPSERVER_DB, buffer );
			break;
		case EN_APPSERVER_LOG:
			sendToServer( EN_APPSERVER_LOG, buffer );
			break;
		}
	}

	void XAppServer::onProcessRegPacket( XAsioRecvPacket& recv )
	{
		std::string name;
		int type;
		int id;
		recv >> type >> id >> name;
		if ( type < EN_APPSERVER_COUNT )
		{
			onLog( outputString( "server=%s register type=%d id=%d", name.c_str(), type, recv.getFromID() ) );
			MAP_APPSERVER_SESSION& container = m_mapAppSrvSession[type];
			XAsioTCPSrvSession* pSession = m_ptrServer->getSession( recv.getFromID() );
			if ( pSession )
			{
				container.insert( std::make_pair( recv.getFromID(), pSession ) );
			}			
		}
	}
	
	void XAppServer::onProcessGMPacket( XAsioRecvPacket& recv )
	{
	}

	void XAppServer::onProcessHeartBeatPacket( XAsioRecvPacket& recv )
	{
	}

	void XAppServer::onProcessEchoPacket( XAsioRecvPacket& recv )
	{
	}
}