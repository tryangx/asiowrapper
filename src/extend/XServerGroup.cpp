#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{
#define DEFAULT_THREAD		1

#define DEFAULT_PORT		8000

	stServerEndPoint::stServerEndPoint() : _ip( NULL ), _port( 0 ), _type( EN_APPSERVER_COUNT ) {}
	stServerEndPoint::stServerEndPoint( const char* ip, short port, enAppServerType type ) : _ip( ip ), _port( port ), _type( type ) {}
	
	stServerConfig::stServerConfig() : _listenPort( 0 )
	{
	}

	void stServerConfig::addEndPoint( stServerEndPoint& ep )
	{
		_mapSrvEndPoint[ep._type] = ep;
	}

	//----------≤‚ ‘
	//  DB <- WORLD  -> LOG
	//			|
	//        GATE
	//			|
	//		  CLIENT

	void stServerConfig::testGateConfig()
	{
		_sName = "[gate]";
		_serverType = EN_APPSERVER_GATE;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		//addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_CENTER, EN_APPSERVER_CENTER ) );
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_WORLD, EN_APPSERVER_WORLD ) );
	}
	void stServerConfig::testWorldConfig()
	{
		_sName = "[world]";
		_serverType = EN_APPSERVER_WORLD;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_DB, EN_APPSERVER_DB ) );
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_LOG, EN_APPSERVER_LOG ) );
	}
	void stServerConfig::testDBConfig()
	{
		_sName = "[db]";
		_serverType = EN_APPSERVER_DB;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		//addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_CENTER, EN_APPSERVER_CENTER ) );
	}
	void stServerConfig::testLogConfig()
	{
		_sName = "[log]";
		_serverType = EN_APPSERVER_LOG;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		//addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_CENTER, EN_APPSERVER_CENTER ) );
	}

	void stServerConfig::testCenterConfig()
	{
		_sName = "[center]";
		_serverType = EN_APPSERVER_CENTER;
		_listenPort = DEFAULT_PORT + _serverType;
		_mapSrvEndPoint.clear();
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_WORLD, EN_APPSERVER_WORLD ) );
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_GATE, EN_APPSERVER_GATE ) );
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_LOG, EN_APPSERVER_LOG ) );
		addEndPoint( stServerEndPoint( "localhost", DEFAULT_PORT + EN_APPSERVER_DB, EN_APPSERVER_DB ) );
	}

	//--------Connector Server

	XConnector::XConnector( XAsioService& service ) : m_ioService( service ), XClient( service )
	{
	}

	void XConnector::setServerId( int id )
	{
		m_iServerId = id;
	}

	void XConnector::onConnect( TcpSessionPtr session )
	{
		XClient::onConnect( session );
		
		XAsioSendPacket packet( 0 );
		packet.getHeader()->setOp( EN_POP_HEARTBEAT );
		send( packet );
	}
	void XConnector::onRecv( XAsioBuffer& buff )
	{
		XClient::onRecv( buff );
	}

	bool XConnector::connect( stServerEndPoint& ep )
	{
		if ( isConnected() )
		{
			return false;
		}
		setAddress( ep._ip, ep._port );
		XClient::connect();
		m_ioService.startService( getService().get(), 1 );
/*
		if ( m_ptrClient && !m_ptrClient->isConnected() )
		{
			return false;
		}
		if ( !m_ptrClient )
		{
			m_ptrClient = XClientPtr( new XClient( m_ioService ) );
		}
		m_ptrClient->setClientId( m_iServerId );
		m_ptrClient->setAddress( ep._ip, ep._port );
		m_ptrClient->connect();		
		m_ioService.startService( m_ptrClient->getService().get(), 1 );
		*/
		return true;
	}

	void XConnector::send( XAsioBuffer& buff )
	{
		XClient::send( buff );
	}

	void XConnector::send( XAsioSendPacket& packet )
	{
		XAsioBuffer buff;
		packet.output( buff );
		XClient::send( buff );
	}

	//--------App server
	XAppServer::XAppServer() : m_bConfigLoaded( false ), m_funcLogHandler( nullptr )
	{
	}
	XAppServer::~XAppServer()
	{
	}

	void XAppServer::setIoService( boost::shared_ptr<class XAsioService>& ioService )
	{
		m_ptrService = ioService;
	}

	void XAppServer::queryAddress( std::string& ip, int& port )
	{
		ip = "localhost";
		port = m_serverConfig._listenPort;
	}

	bool XAppServer::loadConfig( stServerConfig* pConfig )
	{
		if ( !pConfig )
		{
			return false;
		}
		m_bConfigLoaded = true;
		m_serverConfig = *pConfig;
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
		m_ptrServer->setLogHandler( std::bind( &XAppServer::onLog, this, std::placeholders::_1 ) );
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
		std::unordered_map<int,stServerEndPoint>::iterator it = m_serverConfig._mapSrvEndPoint.begin();
		for ( ; it != m_serverConfig._mapSrvEndPoint.end(); it++ )
		{
			stServerEndPoint& ep = it->second;
			if ( ep._port != 0 )
			{
				XConnector* pConn = getConnector( ep._type );
				if ( !pConn )
				{
					m_ptrConnectors[ep._type] = XConnectorPtr( new XConnector( *m_ptrService.get() ) );
					pConn = getConnector( ep._type );
					pConn->setServerId( ep._type );
				}
				pConn->connect( ep );
				onLog( outputString( "conn %s %d", ep._ip, ep._port ) );
			}
		}
		return true;
	}

	bool XAppServer::stopServer()
	{
		std::unordered_map<int,stServerEndPoint>::iterator it = m_serverConfig._mapSrvEndPoint.begin();
		for ( ; it != m_serverConfig._mapSrvEndPoint.end(); it++ )
		{
			stServerEndPoint& ep = it->second;
			if ( ep._port != 0 )
			{
				XConnector* pConn = getConnector( ep._type );
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
		m_processThread = thread( boost::bind( &XAppServer::onProcessPacketThread, this ) );
	}

	void XAppServer::onProcessPacketThread()
	{
		XAsioRecvPacket packet;
		while( m_ptrServer->isStarted() )
		{
			if ( m_ptrServer->queryRecvPacket( packet ) )
			{
				onProcessPacket( packet );
			}
		}
	}
	XConnector* XAppServer::getConnector( enAppServerType type )
	{
		if ( m_ptrConnectors[type] )
		{
			return m_ptrConnectors[type].get();
		}
		return nullptr;
	}
	void XAppServer::sendToServer( enAppServerType type, XAsioBuffer& buffer )
	{
		XConnector* pConn = getConnector( type );
		if ( pConn )
		{
			pConn->send( buffer );
		}
	}
	void XAppServer::sendToServer( enAppServerType type, XAsioSendPacket& packet )
	{
		XAsioBuffer buff;
		packet.output( buff );
		sendToServer( type, buff );
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
	void XAppServer::onAccept( XServerSession* pSession )
	{
		onLog( outputString( "accept %d", m_serverConfig._listenPort ) );
	}

	void XAppServer::onProcessPacket( XAsioRecvPacket& recv )
	{
		unsigned char op = recv.getHeader()->getOp();
		if ( op == EN_POP_CMD )
		{
			processCmdPacket( recv );
			onLog( outputString( "recv cmd dest %d", recv.getHeader()->getDestID() ) );
		}
		else if ( op == EN_POP_MSG )
		{
			onProcessMsgPacket( recv );
			onLog( outputString( "recv msg type %d", recv.getHeader()->getType() ) );
		}
		else if ( op == EN_POP_HEARTBEAT )
		{
			onLog( "recv heartbeat" );
		}
	}

	void XAppServer::processCmdPacket( XAsioRecvPacket& recv )
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
}