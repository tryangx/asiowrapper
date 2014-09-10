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

		EN_APPSERVER_UNKNOW	= 255,
	};

	//服务器命令类型
	enum enServerCmdOpType
	{
		//无效
		EN_SRVCMDOP_NONE,

		//转发
		EN_SRVCMDOP_REDIRECT,
	};

	/**
	 * 服务器端点
	 */
	struct XGAME_API stAppServerEndPoint
	{
		stAppServerEndPoint();
		stAppServerEndPoint( const char* ip, short port, enAppServerType type );

		const char*			_ip;
		short				_port;
		enAppServerType		_type;
	};

	/**
	 * 应用服务器配置
	 */
	struct XGAME_API stAppServerConfig
	{
		//需要连接的服务器
		std::tr1::unordered_map<int,stAppServerEndPoint>	_mapSrvEndPoint;

		//需要侦听的端口
		short				_listenPort;

		//当前服务器类型
		enAppServerType		_serverType;

		//服务器名称
		std::string			_sName;

		stAppServerConfig();

		//clone data
		void		clone( stAppServerConfig& config );

		//add endpoint
		void		addEndPoint( stAppServerEndPoint& ep );

		//only for debug		
		void		testGateConfig();
		void		testWorldConfig();
		void		testDBConfig();
		void		testLogConfig();
		void		testCenterConfig();
	};
	
	typedef boost::shared_ptr<class XServer>	XServerPtr;
	
	typedef boost::shared_ptr<class XClient>	XClientPtr;

	/**
	 * 用于S2S间的连接
	 */
	class XGAME_API XAppConnector : public XClient
	{
	public:
		XAppConnector( XAsioService& service );
		~XAppConnector();

		/**
		 * 得到连接的编号
		 * 用于连接多个同类型服务器，比如GATE连若干GAME
		 */
		void			setConnectorId( unsigned int id );

		/**
		 * 设置连接的宿主服务器类型
		 * 即本身所处的应用服务器
		 */
		void			setServerType( enAppServerType type );

		/**
		 * 尝试连接
		 */
		bool			connect( stAppServerEndPoint& ep );

		/**
		 * 发送
		 */
		void			send( XAsioBuffer& buff );
		void			send( XAsioSendPacket& packet );

	protected:
		/**
		 * 连接上的处理
		 */
		virtual void	onConnect( TcpSessionPtr session );
		/**
		 * 接收到的处理
		 */
		virtual void	onRecv( XAsioBuffer& buff );
		/**
		 * 重连的回调处理
		 */
		virtual void	onReconnectCallback();

	private:
		XAsioService&			m_ioService;
		
		enAppServerType			m_serverType;

		unsigned int			m_iConnectId;

		stAppServerEndPoint		m_serverEndpoint;
	};

	typedef boost::shared_ptr<class XAppConnector>		XAppConnectorPtr;

	/**
	 * 应用服务器
	 */
	class XGAME_API XAppServer
	{
	public:
		XAppServer();
		~XAppServer();

		/**
		 * 设置io_service
		 */
		void	setIoService( boost::shared_ptr<class XAsioService>& ioService );

		/**
		 * 查询服务器信息
		 */
		void	queryAddress( std::string& ip, int& port );

		/**
		 * 载入配置
		 */
		virtual bool	loadConfig( stAppServerConfig& config );

		/**
		 * 启动服务器
		 */
		virtual bool	startServer();
				
		/**
		 * 停止服务器
		 */
		virtual bool	stopServer();

		/**
		 * 连接其它应用服务器
		 */
		virtual bool	connectAppServer();

		/**
		 * 开始处理封包
		 */
		virtual void	startProcessPacket();

		/**
		 * 设置日志响应
		 */
		void	setLogHandler( std::function<void(const char*)>	handler ) { m_funcLogHandler = handler; }

	protected:
		//-----------会话处理
		XAppConnector*	getConnector( enAppServerType type );

		XServerSession*	getAppServer( enAppServerType type );
				
		//-----------日志处理
		void	onLog( const char* pLog );
		
		//-----------连接处理
		void	onAccept( XServerSession* pSession );

		//-----------发送封包
		void	sendToServer( enAppServerType type, XAsioSendPacket& packet );
		void	sendToServer( enAppServerType type, XAsioBuffer& buffer );

		//-----------接收封包
		/**
		 * 处理封包的纯程
		 */
		void	onProcessPacketThread();
		/**
		 * 处理封包
		 */
		virtual void	onProcessPacket( XAsioRecvPacket& recv );
		/**
		 * 处理协议类封包
		 */
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv ) = 0;		
		/**
		 * 处理命令类封包
		 */
		virtual void	onProcessCmdPacket( XAsioRecvPacket& recv );
		/**
		 * 处理注册类封包
		 */
		virtual void	onProcessRegPacket( XAsioRecvPacket& recv );
		/**
		 * 处理心跳类封包
		 */
		virtual void	onProcessHeartBeatPacket( XAsioRecvPacket& recv );
		/**
		 * 处理GM指令封包
		 */
		virtual void	onProcessGMPacket( XAsioRecvPacket& recv );
		/**
		 * 处理应答类封包
		 */
		virtual void	onProcessEchoPacket( XAsioRecvPacket& recv );

	protected:
		boost::shared_ptr<class XAsioService>	m_ptrService;//		m_service;
		XServerPtr			m_ptrServer;

		bool				m_bConfigLoaded;
		stAppServerConfig	m_serverConfig;		
		
		//服务器对应CONNECTOR ID
		//目前不支持多个服务器连接
		typedef std::map<unsigned int, XServerSession*>		MAP_APPSERVER_SESSION;
		MAP_APPSERVER_SESSION				m_mapAppSrvSession[EN_APPSERVER_COUNT];
		
		//可连接服务器
		XAppConnectorPtr	m_ptrConnectors[EN_APPSERVER_COUNT];

		//封包处理
		boost::thread		m_procPacketThread;

		std::function<void(const char*)>	m_funcLogHandler;

		static std::string	m_sAppServerName[EN_APPSERVER_COUNT];
	};

	class XGAME_API XGateServer : public XAppServer
	{
	public:
		XGateServer();

	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XGAME_API XDBServer : public XAppServer
	{
	public:
		XDBServer();

	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XGAME_API XCenterServer : public XAppServer
	{
	public:
		XCenterServer();
		
	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XGAME_API XWorldServer : public XAppServer
	{
	public:
		XWorldServer();

	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XGAME_API XLogServer : public XAppServer
	{
	public:
		XLogServer();
		
	protected:
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv );
	};

	class XTestClientPool;
	class XGAME_API XTestClient : public XClient
	{
	public:
		XTestClient( XAsioService& io );
	};

	typedef boost::shared_ptr<class XTestClient>	XTestClientPtr;
	
	/**
	 * 客户端连接池，用于压力测试
	 */
	class XGAME_API XTestClientPool
	{
	public:
		XTestClientPool();
		~XTestClientPool();

	public:
		/**
		 * 设置服务器地址
		 */
		void	setAddress( const char* ip, int port );

		/**
		 * 设置最大连接
		 */
		void	setMaxClient( unsigned int limit );

		/**
		 * 清除
		 */
		void	clear();

		/**
		 * 创建客户端连接
		 */
		unsigned int	createClient();

		/**
		 * 关闭连接
		 * @param	id 为0时随机关闭连接，为-1时关闭所有连接，>0时关闭指定ID的连接
		 */
		void	closeClient( int id = 0 );

		/**
		 * 所有已连接客户端一起发送封包
		 */
		void	send( XAsioBuffer& buffer );
		void	send( XAsioSendPacket& packet );
		
		/**
		 * 所有对象都执行一个函数
		 */
		void	forEachClient( std::function<void(XTestClient*)> handler );

		/**
		 * 设置日志处理
		 */
		void	setLogHandler( std::function<void(const char*)>	handler ) { m_funcLogHandler = handler; }

	protected:
		void	onLog( const char* pLog );

		void	onClientConnect( XClient* p );
		void	onClientClose( unsigned int );

	protected:
		XAsioService	m_ioService;

		std::string		m_ip;
		int				m_port;

		typedef std::tr1::unordered_map<unsigned int, XTestClientPtr>	MAP_CLINTPTR;
		MAP_CLINTPTR	m_mapClient;
		MAP_CLINTPTR	m_mapTempClient;
		mutex			m_mutexMap;
		unsigned int	m_allocateId;
		unsigned int	m_maxLimited;
				
		std::function<void(const char*)>	m_funcLogHandler;
	};
}