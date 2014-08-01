#pragma once

#include <boost/container/set.hpp>

#include "XAsioBase.h"

namespace XASIO
{
	class XAsioTimer
	{
	protected:

		//定时器信息
		struct stTimerInfo
		{
			enum enTimerStatus
			{
				TIMER_STOP,			//是否停止
				TIMER_RUN,			//是否运行
			};

			//定时器编号
			unsigned int	_id;
			
			//定时器状态
			enTimerStatus	_enStatus;
			
			//定时器间隔时间
			size_t			_milliseconds;
			
			//玩家数据
			const void*		_pUserData;

			//BOOST timer
			boost::shared_ptr<deadline_timer>	_timer;

			/**
			 * ID判断
			 */
			bool operator <( const stTimerInfo& other ) const { return _id < other._id; }
		};

	public:
		typedef	stTimerInfo					TIMER_TYPE;
		typedef const stTimerInfo			TIMER_CTYPE;
		typedef container::set<TIMER_TYPE>	CONTAINER_TIMER;

	protected:
		XAsioTimer( io_service& ioService );

		virtual ~XAsioTimer();

	public:
		//设置定时器
		void	setTimer( unsigned int id, size_t milliseconds, const void* pUserData );

		//停止定时器
		void	stopTimer( unsigned int id );

		//停止全部定时器
		void	stopAllTimer();

		FOREACH_ALL_MUTEX( m_timerContainer, m_timerMutex );
		FOREVERY_ONE_MUTEX( m_timerContainer, m_timerMutex );

	protected:

		/**
		 * 定时器满足条件的响应
		 * @return 返回真时定时器将继续
		 */
		virtual	bool onTimer( unsigned int id, const void* pUserData );

	protected:
		void	startTimer( TIMER_CTYPE& st );

		void	stopTimer( TIMER_TYPE& st );

		void	onTimerHandler( const error_code& ec, TIMER_CTYPE& st );

	protected:
		io_service&		m_refIoService;

		CONTAINER_TIMER	m_timerContainer;

		boost::mutex	m_timerMutex;
	};
}