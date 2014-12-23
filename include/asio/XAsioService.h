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

//ASIO线程数量
//用于监听，消息收发等
#define XASIO_SERVICE_THREAD_NUM		4
	
	class XAsioServiceController;
	
	/**
	 * ASIO底层接口声明
	 * 基于asio的设计原理，每个client及server均视为一个服务，统一由XAsioServerController管理
	 */
	class XGAME_API XAsioServiceInterface
	{
	protected:
		XAsioServiceInterface( XAsioServiceController& controller );
	
	public:
		~XAsioServiceInterface();
		
		/**
		 * 获取服务器编号
		 */
		unsigned long		getServiceId() const;		
		/**
		 * 设置服务器编号
		 */
		void				setServiceId( unsigned long id );
		/**
		 * 服务是否启动
		 */
		bool				isStarted() const;
		/**
		 * 启动服务
		 */
		void				startService();
		/**
		 * 停止服务
		 */
		void				stopService();

		/**
		 * 得到服务控制器
		 */
		XAsioServiceController&		getController();
				
	protected:	
		XAsioServiceController&		m_controller;
		boost::asio::io_service&	m_ioService;
		boost::asio::strand			m_strand;
		
		/**
		 * 服务的编号
		 */
		unsigned long				m_dwServiceId;
		/**
		 * 服务是否已启动
		 */
		bool						m_bIsStarted;
		/**
		 * 服务是否关闭中
		 */
		bool						m_bIsClosing;
	};
		
	/**
	 * 客户端接口声明
	 */
	class XGAME_API XAsioClientInterface : public XAsioServiceInterface
	{
	protected:
		XAsioClientInterface( XAsioServiceController& controller );
	
	public:
		~XAsioClientInterface();

		/**
		 * 连接到指定地址和端口
		 * @param	host	连接的地址
		 * @param	port	连接的端口
		 */
		virtual void	connect( const std::string& host, uint16_t port ) = 0;

		/**
		 * 连接到指定地址和协议
		 * @param	host	连接的地址
		 */
		virtual void	connect( const std::string& host, const std::string& protocol ) = 0;
	};

	/**
	 * 服务器接口声明
	 */
	class XGAME_API XAsioServerInterface : public XAsioServiceInterface
	{
	protected:
		XAsioServerInterface( XAsioServiceController& controller );

	public:
		/**
		 * 开始侦听连接
		 * @param	threadNum	用于侦听的线程数量
		 * @param	port		侦听的端口
		 */
		virtual void	startAccept( int threadNum, uint16_t port ) = 0;
	};

	/**
	 * 服务池
	 */
	class XGAME_API XAsioServicePool
	{
	public:
		XAsioServicePool();

	public:
		/**
		 * 是否启动
		 */
		bool	isRunning() const;

		/**
		 * 初始化
		 * @param	poolSize	池大小
		 */
		void	init( size_t poolSize );

		/**
		 * 启用池中的服务
		 */
		void	start();

		/**
		 * 停止池中的服务
		 */
		void	stop();

		/**
		 * 重置
		 */
		void	reset();

		/**
		 * 获取asio接口
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
	 * ASIO服务控制器
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
		 * 服务管理器是否启动
		 */
		bool	isStarted() const;

		/**
		 * 服务管理器是否运行中
		 */
		bool	isRunning() const;

		/**
		 * 注册服务
		 */
		void	registerService( SERVICE_OBJECT service );

		/**
		 * 启动全部服务
		 */
		void	startAllServices( int threadNum = XASIO_SERVICE_THREAD_NUM );
		
		/**
		 * 启动全部服务
		 */
		//停止全部服务
		void	stopAllServices();

		/**
		 * 启动全部服务
		 */
		//强制停止全部服务
		void	forceStopAllServices();

		/**
		 * 启动全部服务
		 */
		//清除全部服务并重置
		void	clearAllServices();

		//------------------------------------------
		//	单个服务管理

		/**
		 * 启动全部服务
		 */
		//获取服务
		SERVICE_OBJECT	getService( unsigned long dwServiceId );

		/**
		 * 启动全部服务
		 */
		//启动单个服务
		void	startService( SERVICE_OBJECT service, unsigned int threadNum = XASIO_SERVICE_THREAD_NUM );

		/**
		 * 启动全部服务
		 */
		//停止单个服务
		void	stopService( SERVICE_OBJECT service );

		/**
		 * 移除指定服务（并停止）
		 */
		void	removeService( SERVICE_OBJECT service );
		/**
		 * 移除指定ID的服务（并停止）
		 */
		void	removeService( unsigned long dwServiceId );

		/**
		 * 获取asio_id接口
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
		 * 控制器是否启动
		 */
		bool				m_bIsStarted;

		/**
		 * 服务容器
		 */
		SERVICE_CONTAINER	m_serviceContainer;

		/**
		 * ASIO服务
		 */
		io_service			m_ioService;
		IOWORK_PTR			m_ptrIoWork;

		mutex				m_srvMutex;
	};
}