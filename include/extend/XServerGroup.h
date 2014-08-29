#pragma once

#include "../asio/XAsioServer.h"
#include "../asio/XAsioClient.h"
#include "XServerGroupMsg.h"
#include <unordered_map>
#include <functional>

namespace XGAME
{
	//应用服务器类型
	enum enAppServerType
	{
		EN_APPSERVER_GATE,
		
		EN_APPSERVER_CENTER,

		EN_APPSERVER_WORLD,

		EN_APPSERVER_DB,

		EN_APPSERVER_LOG,

		EN_APPSERVER_COUNT,
	};

	//服务器命令类型
	enum enServerCmdOpType
	{
		//无效
		EN_SRVCMDOP_NONE,

		//转发
		EN_SRVCMDOP_REDIRECT,
	};

	struct stServerEndPoint
	{
		stServerEndPoint();
		stServerEndPoint( const char* ip, short port, enAppServerType type );

		const char*			_ip;
		short				_port;
		enAppServerType		_type;
	};

	struct stServerConfig
	{
		//需要连接的服务器
		std::tr1::unordered_map<int,stServerEndPoint>	_mapSrvEndPoint;

		//需要侦听的端口
		short				_listenPort;

		//当前服务器类型
		enAppServerType		_serverType;

		std::string			_sName;

		stServerConfig();

		void		addEndPoint( stServerEndPoint& ep );

		//only for debug		
		void		testGateConfig();
		void		testWorldConfig();
		void		testDBConfig();
		void		testLogConfig();
		void		testCenterConfig();
	};
	
	typedef boost::shared_ptr<class XServer>	XServerPtr;
	
	typedef boost::shared_ptr<class XClient>	XClientPtr;

	class XConnector : public XClient
	{
	public:
		XConnector( XAsioService& service );

		void			setServerId( int id );

		bool			connect( stServerEndPoint& ep );

		void			send( XAsioBuffer& buff );
		void			send( XAsioSendPacket& packet );

	protected:
		virtual void	onConnect( TcpSessionPtr session );
		virtual void	onRecv( XAsioBuffer& buff );

	private:
		XAsioService&	m_ioService;
		
		int				m_iServerId;
	};

	typedef boost::shared_ptr<class XConnector>		XConnectorPtr;

	class XAppServer
	{
	public:
		XAppServer();
		~XAppServer();

		void	setIoService( boost::shared_ptr<class XAsioService>& ioService );

		void	queryAddress( std::string& ip, int& port );

		virtual bool	loadConfig( stServerConfig* pConfig );

		virtual bool	startServer();
				
		virtual bool	stopServer();

		virtual bool	connectAppServer();

		virtual void	startProcessPacket();

		void	setLogHandler( std::function<void(const char*)>	handler ) { m_funcLogHandler = handler; }

	protected:
		XConnector*		getConnector( enAppServerType type );

		void	sendToServer( enAppServerType type, XAsioSendPacket& packet );
		void	sendToServer( enAppServerType type, XAsioBuffer& buffer );

		void	onLog( const char* pLog );

		void	onAccept( XServerSession* pSession );

		void	onProcessPacketThread();

		//-----消息处理

		/**
		 * 处理封包
		 */
		virtual void	onProcessPacket( XAsioRecvPacket& recv );
		
		/**
		 * 处理命令类封包
		 */
		virtual void	processCmdPacket( XAsioRecvPacket& recv );

		/**
		 * 处理协议类封包
		 */
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv ) = 0;
		
	protected:
		bool				m_bConfigLoaded;
		stServerConfig		m_serverConfig;

		boost::shared_ptr<class XAsioService>	m_ptrService;//		m_service;

		XServerPtr			m_ptrServer;

		XConnectorPtr		m_ptrConnectors[EN_APPSERVER_COUNT];

		thread				m_processThread;

		std::function<void(const char*)>	m_funcLogHandler;
	};

	class XGateServer : public XAppServer
	{
	public:
		XGateServer();

	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XDBServer : public XAppServer
	{
	public:
		XDBServer();

	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XCenterServer : public XAppServer
	{
	public:
		XCenterServer();
		
	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XWorldServer : public XAppServer
	{
	public:
		XWorldServer();

	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XLogServer : public XAppServer
	{
	public:
		XLogServer();
		
	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XTestClientPool;
	class XTestClient : public XClient
	{
	public:
		XTestClient( XAsioService& io );
	};

	typedef boost::shared_ptr<class XTestClient>	XTestClientPtr;
	
	class XTestClientPool
	{
	public:
		XTestClientPool();
		~XTestClientPool();

	public:
		void	setAddress( const char* ip, int port );

		void	setMaxConnector( unsigned int limit );

		void	clear();

		void	createClient();

		void	closeClient( int id = 0 );

		void	send( XAsioBuffer& buffer );
		void	send( XAsioSendPacket& packet );
				
		void	forEachClient( std::function<void(XTestClient*)> handler );

		void	onClientConnect( XClient* p );
		void	onClientClose( unsigned int );

		void	setLogHandler( std::function<void(const char*)>	handler ) { m_funcLogHandler = handler; }

		void	onLog( const char* pLog );

	protected:
		typedef std::tr1::unordered_map<unsigned int, XTestClientPtr>	MAPCLINTPTR;
		MAPCLINTPTR		m_mapClient;
		MAPCLINTPTR		m_mapTempClient;
		mutex			m_mutexMap;
		unsigned int	m_allocateId;
		unsigned int	m_maxLimited;

		std::string		m_ip;
		int				m_port;

		XAsioService	m_ioService;

		std::function<void(const char*)>	m_funcLogHandler;
	};
}