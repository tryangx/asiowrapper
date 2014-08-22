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
	class XServer;
	class XServerSession;

	typedef boost::shared_ptr<class XServerSession>		ServerSessionPtr;
	
	typedef XSingleton<XAsioStat>	XAsioStatServerAgent;

	//---------------------------
	//	连接到服务器的会话
	class XGAME_API XServerSession : public boost::enable_shared_from_this<XServerSession>
	{
	public:
		static ServerSessionPtr	create( TcpSessionPtr ptr );
		
		/**
		 * 静态日志控制接口
		 */
		static void		setLogHandler( std::function<void( const char* )> handler );
		static void		setRecvHandler( std::function<void( XServerSession&, XAsioRecvPacket& )> handler );
		static void		onLogHandler( const char* pLog );

	protected:
		static std::function<void( const char* )>						m_sfuncLogHandler;
		static std::function<void( XServerSession&, XAsioRecvPacket& )>	m_sfuncRecvHandler;

	public:
		XServerSession();
		XServerSession( TcpSessionPtr ptr );

		/**
		 * 初始化
		 * @param ptr 指定会话的对象
		 */
		void		init( TcpSessionPtr ptr = nullptr );

		void		release();

		/**
		 * 是否已启动
		 */
		bool		isStarted();

		/**
		 * 是否已停止
		 */
		bool		isOpen();

		/**
		 * 发送
		 */
		void		send( XAsioBuffer& buff );

		/**
		 * 断开
		 */
		void		close();
		
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
	};

	//  服务器
	class XServer //: public XAsioPool<XServerSession>
	{
	public:
		XServer( XAsioService& service );
		~XServer();

		/**
		 * 获取服务
		 */
		TcpServerPtr		getService();

		/**
		 * 获取会话
		 */
		ServerSessionPtr	getSession( unsigned int id );
		
		void				closeSession( unsigned int id );

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

		/**
		 * 获取客户端数量
		 */
		size_t		getClientCount();

		//--------接收消息处理------------

		/**
		 * 获取未处理的消息
		 */
		bool		queryRecvPacket( XAsioRecvPacket& packet );
		
	public:
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		/**
		 * 创建会话
		 */
		virtual ServerSessionPtr	createSession();

		unsigned int		queryValidId();
		
		void		onStartServer();
		void		onAccept( TcpSessionPtr );
		void		onCancel();
		void		onSessionClose( size_t id );		
		void		onSessionRecv( XServerSession& session, XAsioRecvPacket& packet );
		void		onLog( const char* pLog );

	protected:
		/**
		 * asio服务对象
		 */
		XAsioService&					m_service;

		/**
		 * 网络接口，用于服务器处理
		 */
		TcpServerPtr					m_ptrTCPServer;

		/**
		 * 连到到服务器会话数据
		 */
		typedef std::tr1::unordered_map<size_t, ServerSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;
		boost::mutex					m_sessionMutex;

		//侦听线程数量
		int								m_iAcceptThreadNum;

		//使用端口
		int								m_iPort;

		//是否启动
		bool							m_bIsStarted;

		//接收消息队列
		std::list<XAsioRecvPacket>		m_listRecvPackets;
		boost::mutex					m_packetMutex;

		std::function<void( const char* )>			m_funcLogHandler;
	};
}