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

		/**
		 * ��ȡ���ӵ�����ID
		 */
		unsigned int	getSessionId() const;

		/**
		 * �������ӵ�����ID
		 * һ������Ӧ�ķ����Զ�����
		 */
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
		void			setReadHandler( std::function<void( XAsioBuffer& )> handler );		
		void			setWriteHandler( std::function<void( size_t )> handler );
		void			setLogHandler( std::function<void( const char* )> handler );
		void			setCloseHandler( std::function<void( size_t )> handler );

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