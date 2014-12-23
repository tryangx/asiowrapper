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
		XClient( XAsioServiceController& controller );
		~XClient();

		/**
		 * 获取服务接口
		 */
		XAsioTCPClient*	getService();
		
		/**
		 * 得到ID
		 */
		unsigned long	getClientId() const;
		/**
		 * 设置ID
		 */
		void			setClientId( unsigned long id );
		
		/**
		 * 是否已连接
		 */
		bool			isConnected() const;

		/**
		 * 设置是否超时断开
		 */
		void			setTimeoutDisconnect( bool enable );

		/**
		 * 设置地址
		 */
		void			setAddress( std::string host = "localhost", int port = 6580 );
		/**
		 * 获取IP
		 */
		const char*		getIp() const;
		/**
		 * 获取端口
		 */
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
		 * 设置连接到服务器时的处理
		 * @param handler	连接成功或失败时的回调处理，参数为空即失败，失败原因可能为超时等
		 */
		void			setConnectHandler( std::function<void( XClient* )> handler );

		/**
		 * 设置关闭时的处理
		 * 用于服务器主动断开连接
		 */
		void			setCloseHandler( std::function<void( unsigned long )> handler );

		/**
		 * 设置接收到数据时的处理
		 */
		void			setRecvHandler( std::function<void( XClient*, XAsioRecvPacket& )> handler );
		
		/**
		 * 测试消息收发
		 */
		void			testEcho();

	protected:
		/**
		 * 主动接收
		 * 重载此函数可控制需要读取的方式，包头等等
		 */
		virtual void	recv();
		
		virtual void	onConnect( TcpSessionPtr session );

		virtual void	onRecv( XAsioBuffer& buff );

		void			onSend( size_t bytesTransferred );

		void			onClose( size_t id );

		void			onLog( const char* pLog );
		
		void			onConnTimeoutCallback( const boost::system::error_code& err );

		void			onDisconnTimeCallback( const boost::system::error_code& err );

	protected:
		/**
		 * asio服务对象
		 */
		XAsioServiceController&		m_controller;
		/**
		 * 网络接口，用于连接服务器
		 */
		TcpClientPtr		m_ptrTCPClient;
		/**
		 * 会话接口，从网络接口中获取
		 */
		TcpSessionPtr		m_ptrSession;

		/**
		 * 编号，用于池管理
		 */
		unsigned long		m_dwClientId;
				
		/**
		 * 连接IP地址
		 */
		std::string			m_sHost;
		/**
		 * 连接端口
		 */
		int					m_iPort;

		/**
		 * 是否连接到服务器
		 */
		bool				m_bIsConnected;

		/**
		 * 是否开启超时断开连接
		 */
		bool				m_bTimeoutDisconnect;
				
		/**
		 * 读取消息相关
		 */		
		bool				m_bReadHeader;
		XAsioRecvPacket		m_recvPacket;

		/**
		 * 用于连接超时，连接中断处理的计时器
		 */
		deadline_timer		m_timer;
		
		std::function<void( XClient* )>						m_funcConnectHandler;
		std::function<void( unsigned long )>				m_funcCloseHandler;
		std::function<void( XClient*, XAsioRecvPacket& )>	m_funcRecvHandler;

		/**
		 * 测试消息收发
		 */
		bool				m_bTestEcho;
	};
}