/**
 * 网络服务器封装
 */
#pragma once

#include "XAsioTCP.h"

#include <unordered_map>

namespace XASIO
{
	class XServer;
	class XServerSession;

	typedef boost::shared_ptr<class XServerSession>		ServerSessionPtr;

	//---------------------------
	//	连接到服务器的会话
	class XServerSession : public boost::enable_shared_from_this<XServerSession>
	{
	public:
		static ServerSessionPtr	create( TcpSessionPtr ptr );
		
		/**
		 * 静态日志控制接口
		 */
		static void		setLog( std::function<void( const char* )> handler );
		static void		disableLog();

		/**
		 * 得到所有会话接收的消息长度
		 */
		static size_t	getRecvSize();
		static size_t	getSendSize();

	protected:
		static void		onLogHandler( const char* pLog );

	protected:
		static std::function<void( const char* )>	m_sfuncLogHandler;
		static size_t		m_staSizeRecv;
		static size_t		m_staSizeSend;

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

		/**
		 * 测试发送
		 */
		void		testSend();
		void		sendThread();
		void		sendTestPackage();

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
		TcpSessionPtr		m_tcpSession;

		/**
		* 读取消息相关
		*/
		bool				m_bIsStarted;
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;
		XAsioPackage		m_lastPackage;
		
		/**
		 * 发送测试用
		 */
		boost::shared_ptr<boost::thread>		m_sendThread;

		//friend				XServer;
	};

	//  服务器
	class XServer : public XAsioPool<XServerSession>
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

		/**
		 * 测试发送
		 */
		void		testSend();
		void		testSendDirectly();

	public:
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		unsigned int	queryValidId();

		ServerSessionPtr	createSession();

		void		onStartServer();
		void		onAccept( TcpSessionPtr );
		void		onCancel();
		void		onSessionClose( size_t id );
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

		boost::mutex					m_mutex;

		//侦听线程数量
		int								m_iAcceptThreadNum;

		//使用端口
		int								m_iPort;

		//是否启动
		bool							m_bIsStarted;

		std::function<void( const char* )>			m_funcLogHandler;
	};
}