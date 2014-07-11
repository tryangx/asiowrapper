#include "../include/XAsioService.h"

namespace XASIO
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
		: m_funcLogHandler( nullptr ), m_strand( service ), m_service( service ),
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
	
	XAsioService& XAsioInterface::getIOService() { return m_service; }
	const XAsioService& XAsioInterface::getIOService() const { return m_service; }

	//--------------------------------

	XAsioService::~XAsioService()
	{
	}

	bool XAsioService::isRunning() const { return !stopped(); }

	bool XAsioService::isStarted() const { return m_bIsStarted; }

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
			reset();
			forEachAll( boost::mem_fn( &XAsioInterface::startService ) );
			runIOService( threadNum );
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
					stop();
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

		XASIO::forEachAll( tempCont, boost::bind(&XAsioService::stopAndFree, this, _1) );
	}

	//-------------------------------------------

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

	void XAsioService::onLog( std::string& err )
	{
		mutex::scoped_lock lock( m_srvMutex );
		if ( m_funcLogHandler != nullptr )
		{
			m_funcLogHandler( err );
		}
	}
	void XAsioService::onLog( const char* pInfo )
	{
		onLog( std::string( pInfo ) );
	}

	//---------------------------------------

	bool XAsioService::onServiceException(const std::exception& e)
	{
		return true;
	}

	size_t XAsioService::runIOServiceThread( error_code& ec )
	{
		while (true)
		{
			try { return io_service::run(ec); }
			catch ( const std::exception& e )
			{
				if ( !onServiceException(e) ) return 0;
			}
		}
	}

	void XAsioService::registerService( SERVICE_TYPE service )
	{
		assert( nullptr != service );

		mutex::scoped_lock lock( m_srvMutex );
		m_srvContainer.push_back( service );
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
		run( ec );

		if ( threadNum > 0 )
		{
			tg.join_all();
		}

		m_bIsStarted = false;
	}
}