/**
 * �ײ��������
 *
 * XAsioInterface
 *	�ײ�ӿ�
 * 
 * XAsioService
 *	�����������
 */
#pragma once

#include <boost/container/list.hpp>

#include "XAsioBase.h"

namespace XASIO
{
//ASIO�߳�����
//���ڼ�������Ϣ�շ���
#ifndef XASIO_SERVICE_THREAD_NUM
#define XASIO_SERVICE_THREAD_NUM		8
#endif
	
	class XAsioService;
	/**
	 * ASIO�ײ�ӿ�����
	 */
	class XAsioInterface
	{
	protected:
		XAsioInterface( XAsioService& service );
	
	public:
		~XAsioInterface();
				
		//�����Ƿ�����
		bool				isStarted() const;
		//��������
		void				startService();
		//ֹͣ����
		void				stopService();

		unsigned int		getId() const;
		void				setId( unsigned int id );

		//io_service
		XAsioService&		getIOService();
		const XAsioService& getIOService() const;
		
		/**
		 * ��������ĳ�ʼ��
		 */
		virtual void		init() = 0;
		/**
		 * ֹͣ����ĳ�ʼ��
		 */
		virtual void		release() = 0;
		
	public:
		/**
		 * �����������Ӧ
		 * ����ԭ��Ϊfunction( std::string, size_t )
		 */
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		XAsioService&				m_service;
		boost::asio::strand			m_strand;
		
		unsigned int				m_id;
		bool						m_bIsStarted;

		std::function<void( std::string& )>		m_funcLogHandler;
	};

	//ASIO������������
	class XAsioService : public io_service
	{
	public:
		typedef XAsioInterface*					SERVICE_TYPE;
		typedef const SERVICE_TYPE				SERVICE_CTYPE;
		typedef container::list<SERVICE_TYPE>	CONTAINER_TYPE;

	public:
		XAsioService(void) : m_bIsStarted( false ) {}
		~XAsioService(void);

	public:
		//------------------------------------------
		// status

		//is service started
		bool	isStarted() const;

		//is service still running
		bool	isRunning() const;

		//------------------------------------------
		//  manager

		//����ȫ������
		void	startAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );

		//����ʽ����ȫ������
		void	runAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );
		
		//ֹͣȫ������
		void	stopAllServices();

		//ǿ��ֹͣȫ������
		void	forceStopAllServices();

		//���ȫ����������
		void	clearAllServices();

		//------------------------------------------
		//	�����������

		//��ȡ����
		SERVICE_TYPE	getService( int id );

		//������������
		void	startService( SERVICE_TYPE service, int threadNum = XASIO_SERVICE_THREAD_NUM );

		//ֹͣ��������
		void	stopService( SERVICE_TYPE service );

		//�Ƴ����񣨲�ֹͣ��
		void	removeService( SERVICE_CTYPE service );
		void	removeService( int serviceId );

	public:
		template< typename HANDLER, typename OBJECT >
		void	setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }

	protected:
		void	registerService( SERVICE_TYPE service );
		
		void	stopAndFree( SERVICE_TYPE service  );

		virtual void freeService( SERVICE_TYPE service );

		FOREACH_ALL_MUTEX( m_srvContainer, m_srvMutex );
		FOREVERY_ONE_MUTEX( m_srvContainer, m_srvMutex );
		
		void	onLog( std::string& err );
		void	onLog( const char* pInfo );

	private:		
		void			runIOService( int threadNum );
		size_t			runIOServiceThread( error_code& ec );
		virtual bool	onServiceException(const std::exception& e);
				
	protected:
		//------------------------------------------
		//	member
		bool				m_bIsStarted;

		CONTAINER_TYPE		m_srvContainer;
		mutex				m_srvMutex;

		std::function<void( std::string& )>			m_funcLogHandler;

		friend XAsioInterface;
	};
}