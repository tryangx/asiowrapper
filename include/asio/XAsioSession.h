/**
 * 连接会话接口
 */
#pragma once

#include "XAsioService.h"
#include "XAsioHelper.h"
#include <boost/array.hpp>

namespace XGAME
{
	/**
	 * 会话接口声明
	 */
	class XGAME_API XAsioSession
	{
	public:
		~XAsioSession();

		/**
		 * 获取连接的序列ID
		 */
		unsigned long	getSessionId() const;

		/**
		 * 设置连接的序列ID
		 * 一般由相应的服务自动生成
		 */
		void			setSessionId( unsigned long id );
		
		/**
		 * 接收
		 */
		virtual void	recv() = 0;

		/**
		 * 发送
		 */
		virtual void	send( XAsioBuffer& buffer ) = 0;	
		
		/**
		 * 释放,删除会话相关的回调
		 */
		virtual void	release();

	public:
		/**
		 * 设置接收消息的回调
		 */
		void			setRecvHandler( std::function<void( XAsioBuffer& )> handler );		

		/**
		 * 设置发送消息的回调
		 */
		void			setSendHandler( std::function<void( size_t )> handler );

		/**
		 * 设置关闭时的回调
		 */
		void			setCloseHandler( std::function<void( size_t )> handler );

	protected:
		XAsioSession( XAsioServiceController& controller );

		/**
		 * 重置状态
		 */
		void	resetState();

		/**
		 * 清空缓存
		 */
		void	clearBuffers();

		/**
		 * 收到数据的响应
		 */
		virtual void	onRecvCallback( const boost::system::error_code& err, size_t bytesTransferred );

		/**
		 * 发送数据的响应
		 */
		virtual void	onSendCallback( const boost::system::error_code& err, size_t bytesTransferred );

	protected:
		/**
		 * 服务控制器
		 */
		XAsioServiceController&				m_controller;
		/**
		 * ASIO接口，用于支持池模式
		 */
		boost::asio::io_service&			m_ioService;
		boost::asio::strand					m_strand;		

		/**
		 * 会话编号，作为键值
		 */
		unsigned long						m_dwSessionId;
		
		/**
		 * 接收缓存
		 */
		char								m_recvBuffer[MAX_PACKET_SIZE];
		/**
		 * 发送缓存
		 */
		char								m_sendBuffer[MAX_PACKET_SIZE];

		/**
		 * 接收的回调函数
		 */
		std::function<void( XAsioBuffer& )>	m_funcRecvHandler;
		/**
		 * 读取的回调函数
		 */
		std::function<void( size_t )>		m_funcSendHandler;
		/**
		 * 关闭的回调函数
		 */
		std::function<void( size_t )>		m_funcCloseHandler;
	};
}