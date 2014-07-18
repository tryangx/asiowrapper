/**
 * 底层服务声明
 *
 * XAsioInterface
 *	底层接口
 * 
 * XAsioService
 *	基本服务控制
 */
#pragma once

#include <boost/container/list.hpp>

#include "XAsioBase.h"

namespace XASIO
{
//ASIO线程数量
//用于监听，消息收发等
#ifndef XASIO_SERVICE_THREAD_NUM
#define XASIO_SERVICE_THREAD_NUM		8
#endif
	
	class XAsioService;
	/**
	 * ASIO底层接口声明
	 */
	class XAsioInterface
	{
	protected:
		XAsioInterface( XAsioService& service );
	
	public:
		~XAsioInterface();
				
		//服务是否启动
		bool				isStarted() const;
		//启动服务
		void				startService();
		//停止服务
		void				stopService();

		unsigned int		getId() const;
		void				setId( unsigned int id );

		//io_service
		XAsioService&		getIOService();
		const XAsioService& getIOService() const;
		
		/**
		 * 启动服务的初始化
		 */
		virtual void		init() = 0;
		/**
		 * 停止服务的初始化
		 */
		virtual void		release() = 0;
		
	public:
		/**
		 * 发生错误的响应
		 * 函数原型为function( std::string, size_t )
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

	//ASIO服务器管理器
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

		//启动全部服务
		void	startAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );

		//阻塞式运行全部服务
		void	runAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );
		
		//停止全部服务
		void	stopAllServices();

		//强制停止全部服务
		void	forceStopAllServices();

		//清除全部服务并重置
		void	clearAllServices();

		//------------------------------------------
		//	单个服务管理

		//获取服务
		SERVICE_TYPE	getService( int id );

		//启动单个服务
		void	startService( SERVICE_TYPE service, int threadNum = XASIO_SERVICE_THREAD_NUM );

		//停止单个服务
		void	stopService( SERVICE_TYPE service );

		//移除服务（并停止）
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