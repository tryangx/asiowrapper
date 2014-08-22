/**
 * 连接会话接口
 */
#pragma once

#include "XAsioService.h"
#include "XAsioHelper.h"
#include <boost/array.hpp>

namespace XGAME
{
	//是否使用固定的接收缓存
	//开启后将会使用预分配的缓存接收数据，避免内存碎片
	//如果接收消息超过MAX_MSG_LEN长度时，仍将临时分配内存以接收
#define USE_RECV_BUFFER

	//最大缓存消息数量
#ifndef MAX_BUFFER_NUM
#define MAX_BUFFER_NUM				1024
#endif

	/**
	 * 会话接口声明
	 */
	class XGAME_API XAsioSession
	{
	public:
		~XAsioSession();

		//连接的ID
		unsigned int	getSessionId() const;
		void			setSessionId( unsigned int id );
		
		/**
		 * 接收
		 */
		virtual void	read() = 0;

		/**
		 * 发送
		 */
		virtual void	write( XAsioBuffer& buffer ) = 0;	
		
		/**
		 * 释放,删除会话相关的回调
		 */
		virtual void	release();

	public:
		template< typename HANDLER, typename OBJECT >
		void			setReadHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcReadHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		template< typename HANDLER, typename OBJECT >
		void			setReadCompleteHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcReadCompleteHandler = std::bind( eventHandler, eventHandlerObject ); }
		template< typename HANDLER, typename OBJECT >
		void			setWriteHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcWriteHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }		
		template< typename HANDLER, typename OBJECT >
		void			setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		template< typename HANDLER, typename OBJECT >
		void			setCloseHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcCloseHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }

	protected:
		XAsioSession( XAsioService& service );

		void	resetState();
		void	clearBuffers();

		/**
		 * 收到数据的响应
		 */
		virtual void	onReadCallback( const boost::system::error_code& err, size_t bytesTransferred );

		/**
		 * 发送数据的响应
		 */
		virtual void	onWriteCallback( const boost::system::error_code& err, size_t bytesTransferred );

	protected:
		XAsioService&						m_service;
		boost::asio::io_service&			m_ioService;
		boost::asio::strand					m_strand;		

		/**
		 * 会话编号，作为键值
		 */
		unsigned int						m_sessionId;
		
		char								m_readBuffer[MAX_PACKET_SIZE];
		char								m_sendBuffer[MAX_PACKET_SIZE];

		std::function<void( XAsioBuffer& )>	m_funcReadHandler;
		std::function<void( size_t )>		m_funcWriteHandler;
		std::function<void( size_t )>		m_funcCloseHandler;
		std::function<void( const char* )>	m_funcLogHandler;		
	};
}