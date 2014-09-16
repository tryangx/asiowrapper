/**
 * 网络客户端封装
 */
#pragma once

#include "XAsioTCP.h"
#include "XAsioStat.h"
#include "../util/XSingleton.h"

namespace XGAME
{
	typedef XSingleton<XAsioStat>	XAsioStatClientAgent;

	class XGAME_API XClient
	{
	public:
		XClient( XAsioService& io );
		~XClient();

		/**
		 * 获取服务接口
		 */
		TcpClientPtr	getServicePtr();
		XAsioTCPClient*	getService();
		
		/**
		 * 得到ID
		 */
		unsigned int	getClientId() const;
		void			setClientId( unsigned int id );

		bool			isConnected() const;

		/**
		 * 设置地址
		 */
		void			setAddress( std::string host = "localhost", int port = 6580 );
		const char*		getIp() const;
		unsigned short	getPort() const;
		
		/**
		 * 连接
		 */
		bool			connect();
		/**
		 * 断开连接
		 */
		void			disconnect();
				
		/**
		 * 发送
		 */
		void			send( XAsioBuffer& buff );

		/**
		 * 测试
		 */
		void			testEcho();

		/**
		 * 设置连接到服务器时的处理
		 * @param handler	连接成功或失败时的回调处理，参数为空即失败，失败原因可能为超时等
		 */
		void			setConnectHandler( std::function<void( XClient* )> handler );

		/**
		 * 设置关闭时的处理
		 * 用于服务器主动断开连接
		 */
		void			setCloseHandler( std::function<void( size_t )> handler );

		/**
		 * 设置接收到数据时的处理
		 */
		void			setRecvHandler( std::function<void( XClient*, XAsioRecvPacket& )> handler );

		/**
		 * 设置接收到数据时的处理
		 */
		void			setLogHandler( std::function<void( const char* )> handler );

	protected:
		/**
		 * 释放会话
		 */
		void			release();
		
		/**
		 * 主动接收
		 * 重载此函数可控制需要读取的方式，包头等等
		 */
		virtual void	recv();
		
		virtual void	onConnect( TcpSessionPtr session );
		virtual void	onRecv( XAsioBuffer& buff );
		void			onSend( size_t bytesTransferred );
		void			onResolve();
		void			onClose( size_t id );
		void			onLog( const char* pLog );
		
		void			onConnTimeoutCallback( const boost::system::error_code& err );
		void			onDisconnTimeCallback( const boost::system::error_code& err );

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
		unsigned int		m_iClientId;

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
		 * 用于连接超时，连接中断处理的计时器
		 */
		deadline_timer		m_timer;

		/**
		 * 读取消息相关
		 */
		bool				m_bTestEcho;
		bool				m_bReadHeader;
		XAsioRecvPacket		m_recvPacket;
		
		std::function<void( size_t )>						m_funcCloseHandler;
		std::function<void( XClient*, XAsioRecvPacket& )>	m_funcRecvHandler;
		std::function<void( XClient* )>						m_funcConnectHandler;
		std::function<void( const char* )>					m_funcLogHandler;
	};
}