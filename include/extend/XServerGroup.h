#pragma once

#include "../asio/XAsioServer.h"
#include "../asio/XAsioClient.h"
#include "XServerGroupMsg.h"
#include <unordered_map>
#include <functional>

namespace XGAME
{
	//Ӧ�÷���������
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

	//��������������
	enum enServerCmdOpType
	{
		//��Ч
		EN_SRVCMDOP_NONE,

		//ת��
		EN_SRVCMDOP_REDIRECT,
	};

	/**
	 * �������˵�
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
	 * Ӧ�÷���������
	 */
	struct XGAME_API stAppServerConfig
	{
		//��Ҫ���ӵķ�����
		std::tr1::unordered_map<int,stAppServerEndPoint>	_mapSrvEndPoint;

		//��Ҫ�����Ķ˿�
		short				_listenPort;

		//��ǰ����������
		enAppServerType		_serverType;

		//����������
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
	 * ����S2S�������
	 */
	class XGAME_API XAppConnector : public XClient
	{
	public:
		XAppConnector( XAsioService& service );
		~XAppConnector();

		/**
		 * �õ����ӵı��
		 * �������Ӷ��ͬ���ͷ�����������GATE������GAME
		 */
		void			setConnectorId( unsigned int id );

		/**
		 * �������ӵ���������������
		 * ������������Ӧ�÷�����
		 */
		void			setServerType( enAppServerType type );

		/**
		 * ��������
		 */
		bool			connect( stAppServerEndPoint& ep );

		/**
		 * ����
		 */
		void			send( XAsioBuffer& buff );
		void			send( XAsioSendPacket& packet );

	protected:
		/**
		 * �����ϵĴ���
		 */
		virtual void	onConnect( TcpSessionPtr session );
		/**
		 * ���յ��Ĵ���
		 */
		virtual void	onRecv( XAsioBuffer& buff );
		/**
		 * �����Ļص�����
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
	 * Ӧ�÷�����
	 */
	class XGAME_API XAppServer
	{
	public:
		XAppServer();
		~XAppServer();

		/**
		 * ����io_service
		 */
		void	setIoService( boost::shared_ptr<class XAsioService>& ioService );

		/**
		 * ��ѯ��������Ϣ
		 */
		void	queryAddress( std::string& ip, int& port );

		/**
		 * ��������
		 */
		virtual bool	loadConfig( stAppServerConfig& config );

		/**
		 * ����������
		 */
		virtual bool	startServer();
				
		/**
		 * ֹͣ������
		 */
		virtual bool	stopServer();

		/**
		 * ��������Ӧ�÷�����
		 */
		virtual bool	connectAppServer();

		/**
		 * ��ʼ������
		 */
		virtual void	startProcessPacket();

		/**
		 * ������־��Ӧ
		 */
		void	setLogHandler( std::function<void(const char*)>	handler ) { m_funcLogHandler = handler; }

	protected:
		//-----------�Ự����
		XAppConnector*	getConnector( enAppServerType type );

		XServerSession*	getAppServer( enAppServerType type );
				
		//-----------��־����
		void	onLog( const char* pLog );
		
		//-----------���Ӵ���
		void	onAccept( XServerSession* pSession );

		//-----------���ͷ��
		void	sendToServer( enAppServerType type, XAsioSendPacket& packet );
		void	sendToServer( enAppServerType type, XAsioBuffer& buffer );

		//-----------���շ��
		/**
		 * �������Ĵ���
		 */
		void	onProcessPacketThread();
		/**
		 * ������
		 */
		virtual void	onProcessPacket( XAsioRecvPacket& recv );
		/**
		 * ����Э������
		 */
		virtual void	onProcessMsgPacket( XAsioRecvPacket& recv ) = 0;		
		/**
		 * ������������
		 */
		virtual void	onProcessCmdPacket( XAsioRecvPacket& recv );
		/**
		 * ����ע������
		 */
		virtual void	onProcessRegPacket( XAsioRecvPacket& recv );
		/**
		 * ������������
		 */
		virtual void	onProcessHeartBeatPacket( XAsioRecvPacket& recv );
		/**
		 * ����GMָ����
		 */
		virtual void	onProcessGMPacket( XAsioRecvPacket& recv );
		/**
		 * ����Ӧ������
		 */
		virtual void	onProcessEchoPacket( XAsioRecvPacket& recv );

	protected:
		boost::shared_ptr<class XAsioService>	m_ptrService;//		m_service;
		XServerPtr			m_ptrServer;

		bool				m_bConfigLoaded;
		stAppServerConfig	m_serverConfig;		
		
		//��������ӦCONNECTOR ID
		//Ŀǰ��֧�ֶ������������
		typedef std::map<unsigned int, XServerSession*>		MAP_APPSERVER_SESSION;
		MAP_APPSERVER_SESSION				m_mapAppSrvSession[EN_APPSERVER_COUNT];
		
		//�����ӷ�����
		XAppConnectorPtr	m_ptrConnectors[EN_APPSERVER_COUNT];

		//�������
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
	 * �ͻ������ӳأ�����ѹ������
	 */
	class XGAME_API XTestClientPool
	{
	public:
		XTestClientPool();
		~XTestClientPool();

	public:
		/**
		 * ���÷�������ַ
		 */
		void	setAddress( const char* ip, int port );

		/**
		 * �����������
		 */
		void	setMaxClient( unsigned int limit );

		/**
		 * ���
		 */
		void	clear();

		/**
		 * �����ͻ�������
		 */
		unsigned int	createClient();

		/**
		 * �ر�����
		 * @param	id Ϊ0ʱ����ر����ӣ�Ϊ-1ʱ�ر��������ӣ�>0ʱ�ر�ָ��ID������
		 */
		void	closeClient( int id = 0 );

		/**
		 * ���������ӿͻ���һ���ͷ��
		 */
		void	send( XAsioBuffer& buffer );
		void	send( XAsioSendPacket& packet );
		
		/**
		 * ���ж���ִ��һ������
		 */
		void	forEachClient( std::function<void(XTestClient*)> handler );

		/**
		 * ������־����
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