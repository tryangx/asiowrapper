#include "../../include/asio/XAsioService.h"

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

	XAsioInterface::XAsioInterface( XAsioService& service )
		: m_funcLogHandler( nullptr ), 
		m_service( service ), m_ioService( service.getIOService() ), m_strand( m_ioService ),
		m_bIsStarted( false ), m_id( 0 )
	{
		m_service.registerService( this );
	}

	XAsioInterface::~XAsioInterface()
	{
		m_funcLogHandler = nullptr;
	}

	bool XAsioInterface::isStarted() const { return m_bIsStarted; }

	void XAsioInterface::startService() { if ( !m_bIsStarted ) { m_bIsStarted = true; init(); } }	
	void XAsioInterface::stopService() { if ( m_bIsStarted ) { m_bIsStarted = false; release(); } }
	
	unsigned int XAsioInterface::getId() const { return m_id; }
	void XAsioInterface::setId( unsigned int id ) { m_id = id; }
	
	XAsioService& XAsioInterface::getService() { return m_service; }
	const XAsioService& XAsioInterface::getService() const { return m_service; }

	//-------------------------------------------

	XAsioServicePool::XAsioServicePool( size_t poolSize ) : m_index( 0 ), m_bIsStarted( false )
	{
		if ( poolSize == 0 )
		{
			throw std::runtime_error( "io_service_pool size is 0 ");
		}

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
	XAsioService::XAsioService() : m_bIsStarted( false )
	{
	}

	XAsioService::~XAsioService()
	{
	}

	bool XAsioService::isStarted() const { return m_bIsStarted; }

	bool XAsioService::isRunning() const { return !m_ioService.stopped(); }
	
	void XAsioService::startAllServices( int threadNum )
	{
		if ( !isStarted() )
		{
			thread( boost::bind( &XAsioService::runAllServices, this, threadNum ) );
			int numLoop = XASIO_SERVICE_START_COUNTER;
			while( --numLoop >= 0 && !isStarted() )
			{
				this_thread::sleep( get_system_time() + posix_time::milliseconds( XASIO_SERVICE_THREAD_INTERVAL ) );
			}
		}
	}

	void XAsioService::runAllServices( int threadNum )
	{
		if ( !isStarted() )
		{
			m_ioService.reset();
			forEachAll( boost::mem_fn( &XAsioInterface::startService ) );
			runIOService( XASIO_SERVICE_THREAD_NUM );
		}
	}

	void XAsioService::stopAllServices()
	{
		if ( isStarted() )
		{
			forEachAll( boost::mem_fn( &XAsioInterface::stopService ) );
			while( isStarted() )
			{
				this_thread::sleep( get_system_time() + posix_time::milliseconds( XASIO_SERVICE_THREAD_INTERVAL ) );
			}
		}
	}

	void XAsioService::forceStopAllServices()
	{
		if ( isStarted() )
		{
			forEachAll( boost::mem_fn( &XAsioInterface::stopService ) );

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

	void XAsioService::clearAllServices()
	{
		mutex::scoped_lock lock( m_srvMutex );
		CONTAINER_TYPE	tempCont;
		m_srvContainer.splice( std::end( tempCont ), m_srvContainer );
		lock.unlock();

		XGAME::forEachAll( tempCont, boost::bind(&XAsioService::stopAndFree, this, _1) );
	}

	void XAsioService::registerService( SERVICE_TYPE service )
	{
		assert( nullptr != service );

		mutex::scoped_lock lock( m_srvMutex );
		m_srvContainer.push_back( service );
	}

	XAsioService::SERVICE_TYPE XAsioService::getService( int serviceId )
	{
		mutex::scoped_lock lock( m_srvMutex );
		CONTAINER_TYPE::iterator iter = std::find_if( std::begin( m_srvContainer ), std::end( m_srvContainer ), [=]( SERVICE_CTYPE& item ) { return serviceId == item->getId(); } );
		return iter != std::end( m_srvContainer ) ? *iter : nullptr;
	}

	void XAsioService::startService( SERVICE_TYPE service, int threadNum )
	{
		assert( nullptr != service );
		if ( isStarted() )
		{
			service->startService();
		}
		else
		{
			startAllServices( threadNum );
		}
	}

	void XAsioService::stopService( SERVICE_TYPE service )
	{
		assert( nullptr != service );
		service->stopService();
	}

	void XAsioService::removeService( SERVICE_TYPE service )
	{
		assert( nullptr != service );
		mutex::scoped_lock lock( m_srvMutex );
		m_srvContainer.remove( service );
		lock.unlock();
		stopAndFree( service );
	}

	void XAsioService::removeService( int srvId )
	{
		mutex::scoped_lock lock( m_srvMutex );
		CONTAINER_TYPE::iterator iter = std::find_if( std::begin( m_srvContainer ), std::end( m_srvContainer ), [=]( SERVICE_CTYPE& item ) { return srvId == item->getId(); } );
		if ( iter != std::end( m_srvContainer ) )
		{
			SERVICE_TYPE service = *iter;
			m_srvContainer.erase( iter );
			lock.unlock();
			stopAndFree( service );
		}
	}

	void XAsioService::stopAndFree( SERVICE_TYPE service )
	{
		assert( nullptr != service );
		service->stopService();
		freeService( service );
	}

	void XAsioService::freeService( SERVICE_TYPE service )
	{
		//释放服务器需要做的
	}

	void XAsioService::onLog( const char* pLog )
	{
		mutex::scoped_lock lock( m_srvMutex );
		ON_CALLBACK_PARAM( m_funcLogHandler, pLog );
	}

	io_service&	XAsioService::getIOService()
	{
		return m_ioService;
	}

	size_t XAsioService::runIOServiceThread( error_code& ec )
	{
		while (true)
		{
			try { return m_ioService.run(ec); }
			catch ( const std::exception& ) {}
		}
		return 0;
	}

	void XAsioService::runIOService( int threadNum )
	{
		m_bIsStarted = true;

		threadNum--;
		boost::thread_group tg;
		for ( int i = 0; i < threadNum; ++i )
		{
			tg.create_thread( boost::bind( &XAsioService::runIOServiceThread, this, error_code() ) );
		}
		error_code ec;
		m_ioService.run( ec );

		if ( threadNum > 0 )
		{
			tg.join_all();
		}

		m_bIsStarted = false;
	}
}