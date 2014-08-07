#include "../../include/asio/XAsioHelper.h"

namespace XASIO
{
#define MAX_DEBUG_STRING_LENGTH			2048

#define MAX_LOG_BUFFER		4096

	char* outputString( const char* pszFormat, ... )
	{
		static boost::mutex	_mutex;
		static char text[MAX_LOG_BUFFER];
		mutex::scoped_lock lock( _mutex );
		va_list args;
		va_start( args, pszFormat );
		vsnprintf_s( text, MAX_LOG_BUFFER, pszFormat, args );
		va_end( args );
		return text;
	}

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
		if ( !ec && onTimer( st._id, st._pUserData ) && stTimerInfo::TIMER_RUN == st._enStatus )
		{
			startTimer( st );
		}
	}
	
	//------------------------------
	// ÁÙÊ±»º´æ

	XAsioBuffer::stBuffInfo::stBuffInfo( void* pData, size_t size, bool bOwnsData ) 
		: _pData( pData ), _allocatedSize( size ), _dataSize( size ), _bOwnsData( bOwnsData )
	{
	}

	XAsioBuffer::stBuffInfo::~stBuffInfo()
	{
		release();
	}

	void XAsioBuffer::stBuffInfo::release()
	{
		if( _bOwnsData ) 
		{
			free( _pData );
		}
		_dataSize		= 0;
		_allocatedSize	= 0;
		_pData			= nullptr;
		_bOwnsData		= false;
	}
		
	XAsioBuffer::XAsioBuffer() : m_bufData( NULL, 0, false ) {}
	XAsioBuffer::XAsioBuffer( const XAsioBuffer& buffer ) : m_bufData( buffer.m_bufData._pData, buffer.m_bufData._dataSize, false ) {}
	XAsioBuffer::XAsioBuffer( void* pBuffer, size_t size ) : m_bufData( pBuffer, size, false ) {}
	XAsioBuffer::XAsioBuffer( size_t size ) : m_bufData( malloc( size ), size, true ) {}
	XAsioBuffer::~XAsioBuffer()
	{
		m_bufData.release();
	}

	void* XAsioBuffer::getData() { return m_bufData._pData; }
	const void*	XAsioBuffer::getData() const { return m_bufData._pData; }

	size_t XAsioBuffer::getAllocatedSize() const { return m_bufData._allocatedSize; }
	size_t XAsioBuffer::getDataSize() const { return m_bufData._dataSize; }
	void XAsioBuffer::setDataSize( size_t size ) { m_bufData._dataSize = size; }

	void XAsioBuffer::resize( size_t newSize )
	{
		if( !m_bufData._bOwnsData ) return;

		m_bufData._pData = realloc( m_bufData._pData, newSize );
		m_bufData._dataSize = newSize;
		m_bufData._allocatedSize = newSize;
	}

	void XAsioBuffer::detach() { m_bufData._bOwnsData = false; }

	void XAsioBuffer::attach() { m_bufData._bOwnsData = true; }

	void XAsioBuffer::copy( XAsioBuffer& buffer )
	{
		copy( buffer.getData(), buffer.getDataSize() );
	}

	void XAsioBuffer::copy( const void* pData, size_t size )
	{
		if ( size > MAX_PACKAGE_LEN )
		{
			throw std::runtime_error( "out of package length" );
		}
		if ( m_bufData._pData == NULL )
		{
			m_bufData._pData			= malloc( size );
			m_bufData._dataSize			= size;
			m_bufData._allocatedSize	= size;
			m_bufData._bOwnsData		= true;
		}
		if ( m_bufData._allocatedSize < size )
		{
			resize( size );
		}
		else
		{
			m_bufData._dataSize = size;
		}
		memcpy_s( m_bufData._pData, m_bufData._allocatedSize, pData, size );
	}

	void XAsioBuffer::clone( void* pData, size_t size )
	{
		if ( m_bufData._pData && m_bufData._bOwnsData )
		{
			throw std::runtime_error( "data is exist" );
			return;
		}
		m_bufData._pData			= pData;
		m_bufData._allocatedSize	= size;
		m_bufData._dataSize			= size;
		m_bufData._bOwnsData		= false;
	}
	
	void XAsioBuffer::convertToString( std::string& str )
	{
		str = static_cast<const char*>( getData() );
	}

	void XAsioBuffer::convertFromString( std::string& str )
	{
		m_bufData._pData			= &str[0];
		m_bufData._dataSize			= str.size();
		m_bufData._allocatedSize	= m_bufData._dataSize;
	}

	//------------------------------------
	
	XAsioPackageHeader::XAsioPackageHeader() : m_dwFlag(0), m_dwSize(0), m_dwToken(0), m_dwType(0)
	{
	}

	void XAsioPackageHeader::parseFromBuffer( XAsioBuffer& buff )
	{
		memcpy_s( this, XAsioPackageHeader::getHeaderSize(), buff.getData(), buff.getDataSize() );
		if ( m_dwSize > MAX_PACKAGE_LEN )
		{
			throw std::runtime_error( "out of package length" );
		}
	}

	//------------------------------------

	XAsioPackage::XAsioPackage()
	{
		memset( this, 0, XAsioPackage::getSize() );
	}

	bool XAsioPackage::empty()
	{
		return false;
	}

	void XAsioPackage::reset()
	{
		memset( this, 0, XAsioPackage::getSize() );
	}
	
	void XAsioPackage::parseFromBuffer( XAsioBuffer& buff )
	{
		memcpy_s( this, XAsioPackage::getSize(), buff.getData(), XAsioPackage::getSize() );
	}
}