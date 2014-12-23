/**
 * ���ӻỰ�ӿ�
 */
#pragma once

#include "XAsioService.h"
#include "XAsioHelper.h"
#include <boost/array.hpp>

namespace XGAME
{
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
		unsigned long	getSessionId() const;

		/**
		 * �������ӵ�����ID
		 * һ������Ӧ�ķ����Զ�����
		 */
		void			setSessionId( unsigned long id );
		
		/**
		 * ����
		 */
		virtual void	recv() = 0;

		/**
		 * ����
		 */
		virtual void	send( XAsioBuffer& buffer ) = 0;	
		
		/**
		 * �ͷ�,ɾ���Ự��صĻص�
		 */
		virtual void	release();

	public:
		/**
		 * ���ý�����Ϣ�Ļص�
		 */
		void			setRecvHandler( std::function<void( XAsioBuffer& )> handler );		

		/**
		 * ���÷�����Ϣ�Ļص�
		 */
		void			setSendHandler( std::function<void( size_t )> handler );

		/**
		 * ���ùر�ʱ�Ļص�
		 */
		void			setCloseHandler( std::function<void( size_t )> handler );

	protected:
		XAsioSession( XAsioServiceController& controller );

		/**
		 * ����״̬
		 */
		void	resetState();

		/**
		 * ��ջ���
		 */
		void	clearBuffers();

		/**
		 * �յ����ݵ���Ӧ
		 */
		virtual void	onRecvCallback( const boost::system::error_code& err, size_t bytesTransferred );

		/**
		 * �������ݵ���Ӧ
		 */
		virtual void	onSendCallback( const boost::system::error_code& err, size_t bytesTransferred );

	protected:
		/**
		 * ���������
		 */
		XAsioServiceController&				m_controller;
		/**
		 * ASIO�ӿڣ�����֧�ֳ�ģʽ
		 */
		boost::asio::io_service&			m_ioService;
		boost::asio::strand					m_strand;		

		/**
		 * �Ự��ţ���Ϊ��ֵ
		 */
		unsigned long						m_dwSessionId;
		
		/**
		 * ���ջ���
		 */
		char								m_recvBuffer[MAX_PACKET_SIZE];
		/**
		 * ���ͻ���
		 */
		char								m_sendBuffer[MAX_PACKET_SIZE];

		/**
		 * ���յĻص�����
		 */
		std::function<void( XAsioBuffer& )>	m_funcRecvHandler;
		/**
		 * ��ȡ�Ļص�����
		 */
		std::function<void( size_t )>		m_funcSendHandler;
		/**
		 * �رյĻص�����
		 */
		std::function<void( size_t )>		m_funcCloseHandler;
	};
}