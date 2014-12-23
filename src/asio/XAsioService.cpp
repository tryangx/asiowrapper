#include "asio/XAsioService.h"

/*
	XASIO Wrapper

	基于io_service封装一套service控制	
	
	底层接口
	XAsioInterface ->
		XAsioClientInterface -> XAsioTCPClient
		XAsioServerInterface -> XAsioTCPServer

	XAsioSession -> XAsioTCPSession

	XAsioTCPSrvSession include
		XAsioTCPSession

	XServer include
		XAsioTCPSrvSession
		XAsioTCPServer
*/


namespace XGAME
{
	//线程中服务启动停止的间隔时间
#define XASIO_SERVICE_THREAD_INTERVAL		50

	//启动服务的最大重试次数
#define XASIO_SERVICE_START_COUNTER			10

	//停止服务的最大重试次数
#define XASIO_SERVICE_STOP_COUNTER			20

	//-------------------------------------------
	//	接口底层实现

	XAsioServiceInterface::XAsioServiceInterface( XAsioServiceController& controller )
		: m_controller( controller ), m_ioService( m_controller.getAsioIOService() ), m_strand( m_ioService ),
		m_bIsStarted( false ), m_bIsClosing( true ), m_dwServiceId( 0 )
	{
		m_controller.registerService( this );
	}

	XAsioServiceInterface::~XAsioServiceInterface()
	{
	}

	unsigned long XAsioServiceInterface::getServiceId() const
	{
		return m_dwServiceId;
	}

	void XAsioServiceInterface::setServiceId( unsigned long id )
	{
		m_dwServiceId = id;
	}

	bool XAsioServiceInterface::isStarted() const
	{
		return m_bIsStarted;
	}

	void XAsioServiceInterface::startService()
	{
		if ( !m_bIsStarted )
		{
			m_bIsStarted = true;
		}
	}

	void XAsioServiceInterface::stopService()
	{
		if ( m_bIsStarted )
		{
			m_bIsStarted = false;
		}
	}

	XAsioServiceController& XAsioServiceInterface::getController()
	{
		return m_controller;
	}

	//-------------------------------------------
	//	客户端接口实现

	XAsioClientInterface::XAsioClientInterface( XAsioServiceController& controller )
		: XAsioServiceInterface( controller )
	{
	}

	XAsioClientInterface::~XAsioClientInterface()
	{
	}

	//-------------------------------------------
	//	服务端接口实现

	XAsioServerInterface::XAsioServerInterface( XAsioServiceController& controller )
		: XAsioServiceInterface( controller )
	{
	}

	//-------------------------------------------

	XAsioServicePool::XAsioServicePool() : m_index( 0 ), m_bInit( false ), m_bIsStarted( false )
	{
	}

	void XAsioServicePool::init( size_t poolSize )
	{
		if ( !m_bInit )
		{
			return;
		}
		if ( poolSize == 0 )
		{
			throw std::runtime_error( "io_service_pool size is 0 ");
			return;
		}
		m_bInit = true;
		for ( size_t i = 0; i < poolSize; i++ )
		{
			IOSERVICE_PTR io( new asio::io_service );
			m_vIoServices.push_back( io );
			WORK_PTR work( new io_service::work( *io ) );
			m_vWorks.push_back( work );
		}
	}

	void XAsioServicePool::start()
	{
		std::vector<boost::shared_ptr<thread> > vThread;
		for ( size_t i = 0; i < m_vIoServices.size(); ++i)
		{
			boost::shared_ptr<thread> thread( new thread( boost::bind( &asio::io_service::run, m_vIoServices[i] ) ) );
			vThread.push_back( thread );
		}
		for ( size_t i = 0; i < vThread.size(); i++ )
		{
			vThread[i]->join();
		}
		m_bIsStarted = true;
	}

	void XAsioServicePool::reset()
	{
		for ( size_t i = 0; i < m_vIoServices.size(); i++ )
		{
			m_vIoServices[i]->reset();
		}
	}

	void XAsioServicePool::stop()
	{
		for ( size_t i = 0; i < m_vIoServices.size(); i++ )
		{
			m_vIoServices[i]->stop();
		}
		m_bIsStarted = false;
	}

	bool XAsioServicePool::isRunning() const
	{
		for ( size_t i = 0; i < m_vIoServices.size(); i++ )
		{
			if ( !m_vIoServices[i]->stopped() )
			{
				return true;
			}
		}
		return false;
	}

	io_service&	XAsioServicePool::getIOService()
	{
		io_service& io = *m_vIoServices[m_index];
		m_index = m_index + 1 >= m_vIoServices.size() ? 0 : m_index + 1;
		return io;
	}

	//-------------------------------------------

	XAsioServiceController::XAsioServiceController() : m_bIsStarted( false )
	{
	}

	XAsioServiceController::~XAsioServiceController()
	{
		forceStopAllServices();

		m_serviceContainer.clear();
	}

	bool XAsioServiceController::isStarted() const
	{
		return m_bIsStarted;
	}

	bool XAsioServiceController::isRunning() const
	{
		return !m_ioService.stopped();
	}
		
	void XAsioServiceController::startAllServices( int threadNum )
	{
		if ( !isStarted() )
		{
			thread( boost::bind( &XAsioServiceController::runAllServices, this, threadNum ) );

			int numLoop = XASIO_SERVICE_START_COUNTER;
			while( numLoop-- >= 0 && !isStarted() )
			{
				this_thread::sleep( get_system_time() + posix_time::milliseconds( XASIO_SERVICE_THREAD_INTERVAL ) );
			}
		}
	}

	void XAsioServiceController::runAllServices( unsigned int threadNum )
	{
		if ( !isStarted() )
		{
			m_ptrIoWork = boost::shared_ptr<io_service::work>( new io_service::work( getAsioIOService() ) );

			m_ioService.reset();
			
			for ( SERVICE_CONTAINER::iterator it = m_serviceContainer.begin(); it != m_serviceContainer.end(); it++ )
			{
				SERVICE_OBJECT srv = *it;
				srv->startService();
			}

			runIOService( threadNum > 0 ? threadNum : XASIO_SERVICE_THREAD_NUM );
		}
	}

	void XAsioServiceController::stopAllServices()
	{
		if ( isStarted() )
		{
			m_ptrIoWork.reset();

			for ( SERVICE_CONTAINER::iterator it = m_serviceContainer.begin(); it != m_serviceContainer.end(); it++ )
			{
				SERVICE_OBJECT srv = *it;
				srv->stopService();
			}

			while( isStarted() )
			{
				this_thread::sleep( get_system_time() + posix_time::milliseconds( XASIO_SERVICE_THREAD_INTERVAL ) );
			}
		}
	}

	void XAsioServiceController::forceStopAllServices()
	{
		if ( isStarted() )
		{
			m_ptrIoWork.reset();

			for ( SERVICE_CONTAINER::iterator it = m_serviceContainer.begin(); it != m_serviceContainer.end(); it++ )
			{
				SERVICE_OBJECT srv = *it;
				srv->stopService();
			}

			int numLoop = XASIO_SERVICE_STOP_COUNTER;
			while( isStarted() )
			{
				this_thread::sleep( get_system_time() + posix_time::milliseconds( XASIO_SERVICE_THREAD_INTERVAL ) );
				if ( --numLoop <= 0 )
				{
					m_ioService.stop();
					numLoop = 0x7fffffff;
				}
			}
		}
	}

	void XAsioServiceController::clearAllServices()
	{
		mutex::scoped_lock lock( m_srvMutex );
		SERVICE_CONTAINER	tempCont;
		m_serviceContainer.splice( std::end( tempCont ), m_serviceContainer );
		lock.unlock();

		for ( SERVICE_CONTAINER::iterator it = m_serviceContainer.begin(); it != m_serviceContainer.end(); it++ )
		{
			stopAndFree( *it );
		}
	}

	void XAsioServiceController::registerService( SERVICE_OBJECT service )
	{
		assert( nullptr != service );

		mutex::scoped_lock lock( m_srvMutex );
		m_serviceContainer.push_back( service );
	}

	XAsioServiceController::SERVICE_OBJECT XAsioServiceController::getService( unsigned long dwServiceId )
	{
		mutex::scoped_lock lock( m_srvMutex );
		SERVICE_CONTAINER::iterator iter = std::find_if( std::begin( m_serviceContainer ), std::end( m_serviceContainer ),
			[=]( SERVICE_OBJECT& item )
			{
				return dwServiceId == item->getServiceId();
			} );
		return iter != std::end( m_serviceContainer ) ? *iter : nullptr;
	}

	void XAsioServiceController::startService( SERVICE_OBJECT service, unsigned int threadNum )
	{
		assert( nullptr != service );
		if ( isStarted() )
		{
			if ( service )
			{
				service->startService();
			}			
		}
		else
		{
			startAllServices( threadNum );
		}
	}

	void XAsioServiceController::stopService( SERVICE_OBJECT service )
	{
		assert( nullptr != service );
		if ( service )
		{
			service->stopService();
		}		
	}

	void XAsioServiceController::removeService( SERVICE_OBJECT service )
	{
		assert( nullptr != service );
		mutex::scoped_lock lock( m_srvMutex );
		m_serviceContainer.remove( service );
		lock.unlock();
		stopAndFree( service );
	}

	void XAsioServiceController::removeService( unsigned long dwServiceId )
	{
		mutex::scoped_lock lock( m_srvMutex );
		SERVICE_CONTAINER::iterator iter = std::find_if( std::begin( m_serviceContainer ), std::end( m_serviceContainer ),
			[=]( SERVICE_COBJECT& item )
			{
				return dwServiceId == item->getServiceId();
			} );
		if ( iter != std::end( m_serviceContainer ) )
		{
			SERVICE_OBJECT service = *iter;
			m_serviceContainer.erase( iter );
			lock.unlock();
			stopAndFree( service );
		}
	}

	void XAsioServiceController::stopAndFree( SERVICE_OBJECT service )
	{
		assert( nullptr != service );
		if ( service )
		{
			service->stopService();
			freeService( service );
		}		
	}

	void XAsioServiceController::freeService( SERVICE_OBJECT service )
	{
		//释放服务器需要做的
	}

	void XAsioServiceController::onLog( const char* pLog )
	{
		mutex::scoped_lock lock( m_srvMutex );
		XAsioLog::getInstance()->writeLog( pLog );
	}

	io_service&	XAsioServiceController::getAsioIOService()
	{
		return m_ioService;
	}

	size_t XAsioServiceController::runIOServiceThread( error_code& ec )
	{
		try
		{
			return m_ioService.run( ec );
		}
		catch ( const std::exception& )
		{
		}
		return 0;
	}

	void XAsioServiceController::runIOService( unsigned int threadNum )
	{
		m_bIsStarted = true;

		boost::thread_group tg;
		for ( unsigned int i = 0; i < threadNum - 1; ++i )
		{
			tg.create_thread( boost::bind( &XAsioServiceController::runIOServiceThread, this, error_code() ) );
		}

		error_code ec;
		m_ioService.run( ec );

		if ( threadNum > 1 )
		{
			tg.join_all();
		}

		m_bIsStarted = false;
	}
}