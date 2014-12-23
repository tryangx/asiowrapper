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

#include "XApi.h"
#include "XAsioBase.h"
#include "XAsioLog.h"

#include <boost/container/list.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>

namespace XGAME
{
#define DEFAULT_PORT					6805

#define DEFAULT_POOL_SIZE				4

//ASIO�߳�����
//���ڼ�������Ϣ�շ���
#define XASIO_SERVICE_THREAD_NUM		4
	
	class XAsioServiceController;
	
	/**
	 * ASIO�ײ�ӿ�����
	 * ����asio�����ԭ��ÿ��client��server����Ϊһ������ͳһ��XAsioServerController����
	 */
	class XGAME_API XAsioServiceInterface
	{
	protected:
		XAsioServiceInterface( XAsioServiceController& controller );
	
	public:
		~XAsioServiceInterface();
		
		/**
		 * ��ȡ���������
		 */
		unsigned long		getServiceId() const;		
		/**
		 * ���÷��������
		 */
		void				setServiceId( unsigned long id );
		/**
		 * �����Ƿ�����
		 */
		bool				isStarted() const;
		/**
		 * ��������
		 */
		void				startService();
		/**
		 * ֹͣ����
		 */
		void				stopService();

		/**
		 * �õ����������
		 */
		XAsioServiceController&		getController();
				
	protected:	
		XAsioServiceController&		m_controller;
		boost::asio::io_service&	m_ioService;
		boost::asio::strand			m_strand;
		
		/**
		 * ����ı��
		 */
		unsigned long				m_dwServiceId;
		/**
		 * �����Ƿ�������
		 */
		bool						m_bIsStarted;
		/**
		 * �����Ƿ�ر���
		 */
		bool						m_bIsClosing;
	};
		
	/**
	 * �ͻ��˽ӿ�����
	 */
	class XGAME_API XAsioClientInterface : public XAsioServiceInterface
	{
	protected:
		XAsioClientInterface( XAsioServiceController& controller );
	
	public:
		~XAsioClientInterface();

		/**
		 * ���ӵ�ָ����ַ�Ͷ˿�
		 * @param	host	���ӵĵ�ַ
		 * @param	port	���ӵĶ˿�
		 */
		virtual void	connect( const std::string& host, uint16_t port ) = 0;

		/**
		 * ���ӵ�ָ����ַ��Э��
		 * @param	host	���ӵĵ�ַ
		 */
		virtual void	connect( const std::string& host, const std::string& protocol ) = 0;
	};

	/**
	 * �������ӿ�����
	 */
	class XGAME_API XAsioServerInterface : public XAsioServiceInterface
	{
	protected:
		XAsioServerInterface( XAsioServiceController& controller );

	public:
		/**
		 * ��ʼ��������
		 * @param	threadNum	�����������߳�����
		 * @param	port		�����Ķ˿�
		 */
		virtual void	startAccept( int threadNum, uint16_t port ) = 0;
	};

	/**
	 * �����
	 */
	class XGAME_API XAsioServicePool
	{
	public:
		XAsioServicePool();

	public:
		/**
		 * �Ƿ�����
		 */
		bool	isRunning() const;

		/**
		 * ��ʼ��
		 * @param	poolSize	�ش�С
		 */
		void	init( size_t poolSize );

		/**
		 * ���ó��еķ���
		 */
		void	start();

		/**
		 * ֹͣ���еķ���
		 */
		void	stop();

		/**
		 * ����
		 */
		void	reset();

		/**
		 * ��ȡasio�ӿ�
		 */
		io_service&		getIOService();

	protected:
		typedef boost::shared_ptr<asio::io_service>		IOSERVICE_PTR;
		typedef boost::shared_ptr<io_service::work>		WORK_PTR;

		bool							m_bInit;
		bool							m_bIsStarted;

		std::vector<IOSERVICE_PTR>		m_vIoServices;
		std::vector<WORK_PTR>			m_vWorks;
		size_t							m_index;
	};

	/**
	 * ASIO���������
	 */
	class XGAME_API XAsioServiceController
	{
	protected:
		typedef XAsioServiceInterface*					SERVICE_OBJECT;
		typedef const SERVICE_OBJECT					SERVICE_COBJECT;
		typedef std::list<SERVICE_OBJECT>				SERVICE_CONTAINER;
		typedef boost::shared_ptr<io_service::work>		IOWORK_PTR;

	public:
		XAsioServiceController();
		~XAsioServiceController(void);

	public:
		//------------------------------------------
		// status

		/**
		 * ����������Ƿ�����
		 */
		bool	isStarted() const;

		/**
		 * ����������Ƿ�������
		 */
		bool	isRunning() const;

		/**
		 * ע�����
		 */
		void	registerService( SERVICE_OBJECT service );

		/**
		 * ����ȫ������
		 */
		void	startAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );
		
		/**
		 * ����ȫ������
		 */
		//ֹͣȫ������
		void	stopAllServices();

		/**
		 * ����ȫ������
		 */
		//ǿ��ֹͣȫ������
		void	forceStopAllServices();

		/**
		 * ����ȫ������
		 */
		//���ȫ����������
		void	clearAllServices();

		//------------------------------------------
		//	�����������

		/**
		 * ����ȫ������
		 */
		//��ȡ����
		SERVICE_OBJECT	getService( unsigned long dwServiceId );

		/**
		 * ����ȫ������
		 */
		//������������
		void	startService( SERVICE_OBJECT service, unsigned int threadNum = XASIO_SERVICE_THREAD_NUM );

		/**
		 * ����ȫ������
		 */
		//ֹͣ��������
		void	stopService( SERVICE_OBJECT service );

		/**
		 * �Ƴ�ָ�����񣨲�ֹͣ��
		 */
		void	removeService( SERVICE_OBJECT service );
		/**
		 * �Ƴ�ָ��ID�ķ��񣨲�ֹͣ��
		 */
		void	removeService( unsigned long dwServiceId );

		/**
		 * ��ȡasio_id�ӿ�
		 */
		io_service&		getAsioIOService();

	protected:
		void	runAllServices( unsigned int threadNum = XASIO_SERVICE_THREAD_NUM );
		
		void	stopAndFree( SERVICE_OBJECT service  );

		void	freeService( SERVICE_OBJECT service );
				
		void	onLog( std::string& err );
		void	onLog( const char* pInfo );

		void	runIOService( unsigned int threadNum );
		size_t	runIOServiceThread( error_code& ec );

	protected:		
		/**
		 * �������Ƿ�����
		 */
		bool				m_bIsStarted;

		/**
		 * ��������
		 */
		SERVICE_CONTAINER	m_serviceContainer;

		/**
		 * ASIO����
		 */
		io_service			m_ioService;
		IOWORK_PTR			m_ptrIoWork;

		mutex				m_srvMutex;
	};
}