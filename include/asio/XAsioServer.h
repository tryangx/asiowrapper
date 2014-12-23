/**
 * 网络服务器封装
 */
#pragma once

#include "XAsioTCP.h"
#include "XAsioStat.h"
#include "../util/XSingleton.h"

#include <unordered_map>

namespace XGAME
{
	/**
	 * 连接到服务器的会话
	 */
	class XGAME_API XAsioTCPSrvSession : public boost::enable_shared_from_this<XAsioTCPSrvSession>
	{
	public:
		XAsioTCPSrvSession();
		XAsioTCPSrvSession( TcpSessionPtr ptr );
		~XAsioTCPSrvSession();

		/**
		 * 初始化
		 * @param ptr 指定会话的对象
		 */
		void			init( TcpSessionPtr ptr = nullptr );

		void			setStat( XAsioStat* pStat );

		/**
		 * 得到会话ID
		 */
		unsigned int	getSessionId();

		/**
		 * 释放会话相关handler
		 */
		void			release();

		/**
		 * 是否已启动
		 */
		bool			isStarted();

		/**
		 * 是否已停止
		 */
		bool			isOpen();

		/**
		 * 发送
		 */
		void			send( XAsioBuffer& buff );

		/**
		 * 断开
		 */
		void			close();
		
		/**
		 * 是否超时
		 */
		bool			isTimeout();

		//---------回调处理相关
		/**
		 * 接收的处理
		 */
		void			setRecvHandler( std::function<void( XAsioTCPSrvSession*, XAsioRecvPacket& )> handler );
		
	protected:
		void			recv();
	
		void			onLog( const char* pLog );

		virtual void	onClose();
		virtual void	onRecv( XAsioBuffer& buff );
		virtual void	onWrite( size_t bytesTransferred );

	protected:
		/**
		 * 会话接口，用于发送消息
		 */
		TcpSessionPtr		m_ptrSession;

		/**
		* 读取消息相关
		*/
		bool				m_bIsStarted;
		XAsioRecvPacket		m_recvPacket;		
		long long			m_lastTickerTime;

		std::function<void( XAsioTCPSrvSession*, XAsioRecvPacket& )>	m_funcRecvHandler;

		XAsioStat*			m_pStat;
	};

	typedef boost::shared_ptr<class XAsioTCPSrvSession>		TCPSrvSessionPtr;
	//---------------------
	//  服务器
	class XServer // : public XAsioPool<XAsioTCPSrvSession>
	{
	public:
		XServer( XAsioServiceController& controller );
		~XServer();

		/**
		 * 获取服务
		 */
		TcpServerPtr		getService();
		
		/**
		 * 获取IO状态统计器
		 */
		XAsioStat*			getStat();

		/**
		 * 是否启动
		 */
		bool				isStarted();

		/**
		 * 获取会话
		 */
		XAsioTCPSrvSession*		getSession( unsigned int id );
		
		/**
		 * 关闭会话
		 */
		void					closeSession( unsigned int id );

		/**
		 * 设置连接地址
		 */
		void		setAddress( int port );

		/**
		 * 设置侦听的线程数量
		 */
		void		setAcceptThreadNum( int threadNum );

		/**
		 * 启动服务器
		 */
		void		startServer();
		/**
		 * 停止服务器
		 */
		void		stopServer();

		/**
		 * 发送给指定ID玩家
		 */
		void		sendTo( int id, XAsioBuffer& buff );
		/**
		 * 发送给全体
		 */
		void		sendToAll( XAsioBuffer& buff );
		/**
		 * 发送给指定队列
		 */
		void		sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v );

		//--------客户端处理

		/**
		 * 获取客户端数量
		 */
		size_t		getClientCount();

		/**
		 * 刷新客户端
		 * 用于处理超时等情况
		 */
		void		updateClient();

		//--------接收消息处理------------

		/**
		 * 获取未处理的消息
		 */
		bool		queryRecvPacket( XAsioRecvPacket& packet );
		
	public:
		void		setConnectHandler( std::function<void( XAsioTCPSrvSession* )> handler );
		
	protected:
		/**
		 * 创建会话
		 */
		virtual XAsioTCPSrvSession*	createSession();

		unsigned int	queryValidId();
		
		void			onStartServer();
		void			onAccept( TcpSessionPtr );
		void			onCancel();
		void			onSessionClose( size_t id );		
		void			onSessionRecv( XAsioTCPSrvSession* pSession, XAsioRecvPacket& packet );
		void			onLog( const char* pLog );
		void			onTimerUpdateCallback( const boost::system::error_code& err );

	protected:
		/**
		 * asio服务对象
		 */
		XAsioServiceController&			m_controller;

		/**
		 * 网络接口，用于服务器处理
		 */
		TcpServerPtr					m_ptrTCPServer;

		/**
		 * 连到到服务器会话数据
		 */
		typedef std::tr1::unordered_map<size_t, TCPSrvSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;
		boost::mutex					m_sessionMutex;

		//超时计时器
		deadline_timer					m_timerUpdateTimer;

		//侦听线程数量
		int								m_iAcceptThreadNum;

		//使用端口
		int								m_iPort;

		//是否启动
		bool							m_bIsStarted;

		//接收消息队列
		std::list<XAsioRecvPacket>		m_listRecvPackets;
		boost::mutex					m_packetMutex;

		XAsioStat						m_stat;

		std::function<void( XAsioTCPSrvSession* )>		m_funcAcceptHandler;
	};
}