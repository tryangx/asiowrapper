#pragma once

#include <string>

#include <boost/asio.hpp>
#include <functional>

#include "XAsioService.h"
#include "XAsioHelper.h"
#include "XAsioPackage.h"

namespace XASIO
{
	//�Ƿ�ʹ�ù̶��Ľ��ջ���
	//�����󽫻�ʹ��Ԥ����Ļ���������ݣ������ڴ���Ƭ
	//���������Ϣ����MAX_MSG_LEN����ʱ���Խ���ʱ�����ڴ��Խ���
#define USE_RECV_BUFFER
	
	/**
	 * �Ự�ӿ�����
	 */
	class XAsioSessionInterface
	{
	public:
		~XAsioSessionInterface();

		//���ӵ�ID
		unsigned int	getId() const;
		void			setId( unsigned int id );

		//���ӵ�����
		void*			getUserData();
		void			setUserData( void* pData );

		/**
		 * ����
		 */
		virtual void	read() = 0;

		/**
		 * ����
		 */
		virtual void	write( const XAsioBuffer& buffer ) = 0;	

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
		 * �յ����ݵ���Ӧ
		 */
		virtual void	onRead( const boost::system::error_code& err, size_t bytesTransferred );

		/**
		 * �������ݵ���Ӧ
		 */
		virtual void	onWrite( const boost::system::error_code& err, size_t bytesTransferred );

	protected:
		XAsioService&						m_service;
		boost::asio::strand					m_strand;

		/**
		 * �Ự��ţ���Ϊ��ֵ
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


	/**
	 * �ͻ��˽ӿ�����
	 */
	class XAsioClientInterface : public XAsioInterface
	{
	protected:
		XAsioClientInterface( XAsioService& service );
	
	public:
		~XAsioClientInterface();

		/**
		 * ���ӵ�ָ����ַ�Ͷ˿�
		 * @param	host	���ӵĵ�ַ
		 * @param	port	���ӵĶ˿�
		 */
		virtual void	connect( const std::string& host, uint16_t port ) = 0;
		virtual void	connect( const std::string& host, const std::string& protocol ) = 0;
		
	public:
		template< typename HANDLER, typename OBJECT >
		void			setResolveHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcResolveHandler = std::bind( eventHandler, eventHandlerObject ); }
		
	protected:
		std::function<void()>	m_funcResolveHandler;
	};


	/**
	 * �������ӿ�����
	 */
	class XAsioServerInterface : public XAsioInterface
	{
	public:
		/**
		 * ��ʼ��������
		 */
		virtual void	startAccept( int threadNum, uint16_t port ) = 0;

	protected:
		XAsioServerInterface( XAsioService& service );
	};
}