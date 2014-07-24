/**
 * 连接会话接口
 */
#pragma once

#include "XAsioService.h"
#include "XAsioHelper.h"

namespace XASIO
{
	//是否使用固定的接收缓存
	//开启后将会使用预分配的缓存接收数据，避免内存碎片
	//如果接收消息超过MAX_MSG_LEN长度时，仍将临时分配内存以接收
#define USE_RECV_BUFFER

	/**
	 * 会话接口声明
	 */
	class XAsioSessionInterface
	{
	public:
		~XAsioSessionInterface();

		//连接的ID
		unsigned int	getId() const;
		void			setId( unsigned int id );

		//连接的数据
		void*			getUserData();
		void			setUserData( void* pData );

		/**
		 * 接收
		 */
		virtual void	read() = 0;

		/**
		 * 发送
		 */
		virtual void	write( const XAsioBuffer& buffer ) = 0;	
		
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
		XAsioSessionInterface( XAsioService& service );

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
		boost::asio::strand					m_strand;

		/**
		 * 会话编号，作为键值
		 */
		unsigned int						m_id;
		void*								m_pUserData;
		
		size_t								m_bufferSize;		
		boost::asio::streambuf				m_streamRequest;
		boost::asio::streambuf				m_streamResponse;
#ifdef USE_RECV_BUFFER
		char								m_szReadBuffer[MAX_PACKAGE_LEN];
#endif

		std::function<void()>				m_funcReadCompleteHandler;
		std::function<void( XAsioBuffer )>	m_funcReadHandler;
		std::function<void( size_t )>		m_funcWriteHandler;
		std::function<void( size_t )>		m_funcCloseHandler;
		std::function<void( std::string )>	m_funcLogHandler;		
	};
}