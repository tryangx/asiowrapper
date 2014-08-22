#include "asio/XAsioHelper.h"

namespace XGAME
{
	//-----------------------------------------
	//  ¶¨Ê±Æ÷
	XAsioTimer::XAsioTimer( io_service& ioService ) : m_refIoService( ioService )
	{

	}

	XAsioTimer::~XAsioTimer()
	{

	}

	void XAsioTimer::setTimer( unsigned int id, size_t milliseconds, const void* pUserData )
	{
		TIMER_TYPE st = { id };

		mutex::scoped_lock lock( m_timerMutex );		
		auto it = m_timerContainer.find( st );
		if ( it == std::end( m_timerContainer ) )
		{
			it = m_timerContainer.insert( st ).first;
			it->_timer = boost::make_shared<deadline_timer>( m_refIoService );
		}
		it->_enStatus = stTimerInfo::TIMER_RUN;
		it->_milliseconds = milliseconds;
		it->_pUserData = pUserData;
		lock.unlock();

		startTimer( *it );			
	}

	void XAsioTimer::stopTimer( unsigned int id )
	{
		TIMER_TYPE st = { id };

		mutex::scoped_lock lock( m_timerMutex );
		auto it = m_timerContainer.find( st );
		if ( it != std::end( m_timerContainer ) )
		{
			lock.unlock();
			stopTimer( *it );
		}
	}

	void XAsioTimer::stopAllTimer()
	{
		forEachAll( boost::bind( ( void (XAsioTimer::*)(TIMER_TYPE&) )&XAsioTimer::stopTimer, this, _1 ) );
	}

	bool XAsioTimer::onTimer( unsigned int id, const void* pUserData )
	{
		return false;
	}

	void XAsioTimer::startTimer( TIMER_CTYPE& st )
	{
		st._timer->expires_from_now( posix_time::milliseconds( st._milliseconds ) );
		st._timer->async_wait( boost::bind( &XAsioTimer::onTimerHandler, this, placeholders::error, boost::ref(st) ) );
	}

	void XAsioTimer::stopTimer( TIMER_TYPE& st )
	{
		error_code ec;
		st._timer->cancel(ec);
		st._enStatus = stTimerInfo::TIMER_STOP;
	}

	void XAsioTimer::onTimerHandler( const error_code& ec, TIMER_CTYPE& st )
	{
		if ( !ec && onTimer( st._id, st._pUserData ) && stTimerInfo::TIMER_RUN == st._enStatus )
		{
			startTimer( st );
		}
	}	
}