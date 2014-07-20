/**
 * 网络客户端封装
 */
#pragma once

#include "XAsioTCP.h"

namespace XASIO
{
	class XClient
	{
	public:
		/**
		 * 静态日志控制接口
		 */
		static void		setLog( std::function<void( std::string& )> handler );
		static void		disableLog();

		/**
		 * 得到所有客户端发送的长度
		 */
		static size_t	getSendSize();
		/**
		 * 得到所有客户端接收的长度
		 */
		static size_t	getRecvSize();

	protected:
		static void		onLogHandler( std::string& err );
				
	protected:
		static std::function<void( std::string )>	m_sfuncLogHandler;
		/**
		 * 所有客户端发送消息的长度
		 */
		static size_t		m_sizeSend;
		/**
		 * 所有客户端接收消息的长度
		 */
		static size_t		m_sizeRecv;

	public:
		XClient( XAsioService& io );
		~XClient();

		/**
		 * 获取服务接口
		 */
		TcpClientPtr	getService();
		
		/**
		 * 得到ID
		 */
		unsigned int	getId() const;
		void			setId( unsigned int id );

		/**
		 * 设置地址
		 */
		void		setAddress( std::string host = "localhost", int port = DEFAULT_XASIO_PORT );
		
		/**
		 * 连接
		 */
		void		connect();
		/**
		 * 断开连接
		 */
		void		disconnect();
				
		/**
		 * 发送
		 */
		void		send();
		void		send( std::string content );
		void		send( XAsioBuffer& buff );

		/**
		 * 测试
		 */
		void		testSend();		
		void		sendThread();

	protected:
		void		init();
		
		/**
		 * 主动接收
		 * 重载此函数可控制需要读取的方式，包头等等
		 */
		virtual void	recv();
		
		void		onConnect( TcpSessionPtr session );
		void		onRecv( XAsioBuffer& buff );
		void		onRecvComplete();
		void		onSend( size_t bytesTransferred );
		void		onResolve();
		void		onClose( size_t id );
		void		onLogInfo( const char* pInfo );
		void		onLog( std::string& err );

	protected:
		/**
		 * asio服务对象
		 */
		XAsioService&		m_service;
		/**
		 * 网络接口，用于连接服务器
		 */
		TcpClientPtr		m_ptrTCPClient;
		/**
		 * 会话接口，用于收发消息
		 */
		TcpSessionPtr		m_ptrSession;

		/**
		 * 编号，用于池管理
		 */
		unsigned int		m_id;

		/**
		 * 连接信息
		 */
		int					m_iPort;
		std::string			m_sHost;

		/**
		 * 连接状态
		 */
		bool				m_bInit;
		bool				m_bIsConnected;

		/**
		 * 用于连接超时处理的计时器
		 */
		deadline_timer		m_connectTimer;

		/**
		 * 读取消息相关
		 */
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;

		/**
		 * 发送
		 */
		boost::thread		m_sendThread;
	};
}