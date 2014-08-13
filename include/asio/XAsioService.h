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
#include <vector>

#include "XAsioBase.h"

namespace XGAME
{
#define DEFAULT_POOL_SIZE				8

//ASIO�߳�����
//���ڼ�������Ϣ�շ���
#define XASIO_SERVICE_THREAD_NUM		8
	
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

		XAsioService&		getService();
		const XAsioService& getService() const;
		
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
		boost::asio::io_service&	m_ioService;
		boost::asio::strand			m_strand;
		
		size_t						m_id;
		bool						m_bIsStarted;

		std::function<void( const char* )>		m_funcLogHandler;
	};

	class XAsioServicePool
	{
	public:
		explicit XAsioServicePool( size_t poolSize = DEFAULT_POOL_SIZE );

	public:
		bool	isRunning() const;

		void	start();

		void	stop();

		void	reset();

		io_service&		getIOService();

	protected:
		typedef boost::shared_ptr<asio::io_service>		IOSERVICE_PTR;
		typedef boost::shared_ptr<io_service::work>		WORK_PTR;

		bool							m_bIsStarted;

		std::vector<IOSERVICE_PTR>		m_vIoServices;
		std::vector<WORK_PTR>			m_vWorks;
		size_t							m_index;
	};

	//ASIO������������
	class XAsioService
	{
	public:
		typedef XAsioInterface*					SERVICE_TYPE;
		typedef const SERVICE_TYPE				SERVICE_CTYPE;
		typedef container::list<SERVICE_TYPE>	CONTAINER_TYPE;

	public:
		XAsioService();
		~XAsioService(void);

	public:
		//------------------------------------------
		// status

		//is service started
		bool	isStarted() const;

		bool	isRunning() const;

		//------------------------------------------
		//  manager
		void	registerService( SERVICE_TYPE service );

		//����ȫ������
		void	startAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );
		
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

		io_service&		getIOService();

	public:
		template< typename HANDLER, typename OBJECT >
		void	setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }

	protected:
		//����ʽ����ȫ������
		void	runAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );
		
		void	stopAndFree( SERVICE_TYPE service  );

		virtual void freeService( SERVICE_TYPE service );

		FOREACH_ALL_MUTEX( m_srvContainer, m_srvMutex );
		FOREVERY_ONE_MUTEX( m_srvContainer, m_srvMutex );
		
		void	onLog( std::string& err );
		void	onLog( const char* pInfo );

		size_t	runIOServiceThread( error_code& ec );
		void	runIOService( int threadNum );
				
	protected:
		//------------------------------------------
		//	member

		CONTAINER_TYPE		m_srvContainer;
		mutex				m_srvMutex;

		io_service			m_ioService;

		bool				m_bIsStarted;

		std::function<void( const char* )>			m_funcLogHandler;
	};
}