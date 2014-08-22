/**
 * ���ӻỰ�ӿ�
 */
#pragma once

#include "XAsioService.h"
#include "XAsioHelper.h"
#include <boost/array.hpp>

namespace XGAME
{
	//�Ƿ�ʹ�ù̶��Ľ��ջ���
	//�����󽫻�ʹ��Ԥ����Ļ���������ݣ������ڴ���Ƭ
	//���������Ϣ����MAX_MSG_LEN����ʱ���Խ���ʱ�����ڴ��Խ���
#define USE_RECV_BUFFER

	//��󻺴���Ϣ����
#ifndef MAX_BUFFER_NUM
#define MAX_BUFFER_NUM				1024
#endif

	/**
	 * �Ự�ӿ�����
	 */
	class XGAME_API XAsioSession
	{
	public:
		~XAsioSession();

		//���ӵ�ID
		unsigned int	getSessionId() const;
		void			setSessionId( unsigned int id );
		
		/**
		 * ����
		 */
		virtual void	read() = 0;

		/**
		 * ����
		 */
		virtual void	write( XAsioBuffer& buffer ) = 0;	
		
		/**
		 * �ͷ�,ɾ���Ự��صĻص�
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
		 * �յ����ݵ���Ӧ
		 */
		virtual void	onReadCallback( const boost::system::error_code& err, size_t bytesTransferred );

		/**
		 * �������ݵ���Ӧ
		 */
		virtual void	onWriteCallback( const boost::system::error_code& err, size_t bytesTransferred );

	protected:
		XAsioService&						m_service;
		boost::asio::io_service&			m_ioService;
		boost::asio::strand					m_strand;		

		/**
		 * �Ự��ţ���Ϊ��ֵ
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