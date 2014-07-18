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
		
		static void		setLog( std::function<void( std::string& )>& handler );
		static void		disableLog();
		/**
		 * 得到接收到字节数
		 */
		static size_t	getRecvSize();

	public:
		XServerSession();
		XServerSession( TcpSessionPtr ptr );

		/**
		 * 初始化
		 * @param ptr 指定会话的对象
		 */
		void		init( TcpSessionPtr ptr = nullptr );

		/**
		 * 是否已启动
		 */
		bool		isStarted();

		/**
		 * 是否已停止
		 */
		bool		isStoped();

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

	protected:
		/**
		 * 主动接收
		 */
		void			recv();

		void			onLog( std::string& err );
		void			onLogInfo( const char* pInfo );

		virtual void	onClose();
		virtual void	onRecv( XAsioBuffer buff );
		virtual void	onRecvComplete();
		virtual void	onWrite( size_t bytesTransferred );

	protected:
		static void		onLogHandler( std::string& err );

	protected:
		static std::function<void( std::string )>	m_sfuncLogHandler;
		static size_t		m_sizeRecv;

		TcpSessionPtr		m_tcpSession;

		bool				m_bIsStarted;
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;
		
		boost::thread		m_sendThread;

		friend				XServer;
	};

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

		void		testSend();

	public:
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		virtual void init();
		virtual void release();
		/**
		 * 获取有效的ID
		 */
		unsigned int	queryValidId();

		void		onStartServer();
		void		onAccept( TcpSessionPtr );
		void		onCancel();
		void		onSessionClose( size_t id );
		void		onLog( std::string& err );
		void		onLogInfo( const char* pInfo );

	protected:
		XAsioService&					m_service;

		TcpServerPtr					m_ptrTCPServer;

		typedef std::tr1::unordered_map<size_t, ServerSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;

		boost::mutex					m_mutex;
		//config
		int								m_iAcceptThreadNum;
		//address info
		int								m_iPort;
		//status
		bool							m_bIsStarted;

		std::function<void( std::string& )>			m_funcLogHandler;
	};
}