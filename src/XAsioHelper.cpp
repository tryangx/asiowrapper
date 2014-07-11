#include "../include/XAsioHelper.h"

namespace XASIO
{
#define MAX_DEBUG_STRING_LENGTH			2048
	
	//----------------------
	std::string bufferToString( const XAsioBuffer& buffer )
	{
		return std::string( static_cast<const char*>( buffer.getData() ) );
	}

	XAsioBuffer stringToBuffer( std::string& value )
	{
		return XAsioBuffer( &value[ 0 ], value.size() );
	}

	/**
	 * 输出字符串
	 */
	char* outputString( const char* pszFormat, ... )
	{
		static char text[1024];
		va_list args;
		va_start(args, pszFormat);
		_vsnprintf_s( text, sizeof(text), pszFormat, args);
		va_end(args);
		return text;
	}

	//-----------------------------------------
	//  定时器
	XAsioTimer::XAsioTimer( io_service& ioService ) : m_ioService( ioService )
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
			it->_timer = boost::make_shared<deadline_timer>( m_ioService );
		}
		lock.unlock();

		it->_enStatus = stTimerInfo::TIMER_RUN;
		it->_milliseconds = milliseconds;
		it->_pUserData = pUserData;

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
		if ( ec )
		{
			if ( onTimer( st._id, st._pUserData ) && stTimerInfo::TIMER_RUN == st._enStatus )
			{
				startTimer( st );
			}
		}
		else
		{
			//需要处理错误
		}
	}
	
	//------------------------------
	// 临时缓存

	XAsioBuffer::stBuffInfo::stBuffInfo( void* pData, size_t size, bool bOwnsData ) 
		: _pData( pData ), _allocatedSize( size ), _dataSize( size ), _bOwnsData( bOwnsData )
	{
	}

	XAsioBuffer::stBuffInfo::~stBuffInfo()
	{
		if( _bOwnsData ) 
		{
			free( _pData );
			_pData = nullptr;
		}
	}
		
	XAsioBuffer::XAsioBuffer() {}
	XAsioBuffer::XAsioBuffer( void* pBuffer, size_t size ) : m_bufData( new stBuffInfo( pBuffer, size, false ) ) {}
	XAsioBuffer::XAsioBuffer( size_t size ) : m_bufData( new stBuffInfo( malloc( size ), size, true ) ) {}

	void* XAsioBuffer::getData() { return m_bufData->_pData; }
	const void*	XAsioBuffer::getData() const { return m_bufData->_pData; }

	size_t XAsioBuffer::getAllocatedSize() const { return m_bufData->_allocatedSize; }
	size_t XAsioBuffer::getDataSize() const { return m_bufData->_dataSize; }
	void XAsioBuffer::setDataSize( size_t size ) { m_bufData->_dataSize = size; }

	void XAsioBuffer::resize( size_t newSize )
	{
		if( !m_bufData->_bOwnsData ) return;

		m_bufData->_pData = realloc( m_bufData->_pData, newSize );
		m_bufData->_dataSize = newSize;
		m_bufData->_allocatedSize = newSize;
	}

	void XAsioBuffer::copyFrom( const void* pData, size_t size )
	{
		if ( m_bufData == nullptr )
		{
			m_bufData = boost::shared_ptr<stBuffInfo>( new stBuffInfo( malloc( size ), size, true ) );
		}
		if ( m_bufData->_allocatedSize < size )
		{
			resize( size );
		}
		else
		{
			m_bufData->_dataSize = size;
		}
		memcpy( m_bufData->_pData, pData, size );
	}
	
	XAsioPackageHeader::XAsioPackageHeader() : m_dwFlag(0), m_dwPackageSize(0)
	{
	}

	void XAsioPackageHeader::parseFromBuffer( XAsioBuffer& buff )
	{
		memcpy_s( this, XAsioPackageHeader::getSize(), buff.getData(), XAsioPackageHeader::getSize() );
	}

	XAsioPackage::XAsioPackage()
	{
		memset( this, 0, XAsioPackage::getSize() );
	}
	
	void XAsioPackage::parseFromBuffer( XAsioBuffer& buff )
	{
		memcpy_s( this, XAsioPackage::getSize(), buff.getData(), XAsioPackage::getSize() );
	}
}