#include "../../include/asio/XAsioPacket.h"

namespace XGAME
{
	//------------------------------
	// ÁÙÊ±»º´æ

	XAsioBuffer::stBuffInfo::stBuffInfo( void* pData, size_t allocateSize, size_t dataSize, bool bOwnsData )
		: _pData( pData ), _allocatedSize( allocateSize ), _dataSize( dataSize ), _bOwnsData( bOwnsData )
	{
	}

	XAsioBuffer::stBuffInfo::~stBuffInfo()
	{
		release();
	}

	void XAsioBuffer::stBuffInfo::release()
	{
		if ( _bOwnsData ) 
		{
			free( _pData );
		}
		_dataSize		= 0;
		_allocatedSize	= 0;
		_pData			= nullptr;
		_bOwnsData		= false;
	}
		
	XAsioBuffer::XAsioBuffer() : m_bufData( NULL, 0, 0, false ), m_iCursorPos( 0 ) {}
	XAsioBuffer::XAsioBuffer( const XAsioBuffer& buffer ) : m_bufData( buffer.m_bufData._pData, buffer.m_bufData._allocatedSize, buffer.m_bufData._dataSize, false ), m_iCursorPos( 0 ) {}
	XAsioBuffer::XAsioBuffer( void* pBuffer, size_t size ) : m_bufData( pBuffer, size, size, false ), m_iCursorPos( 0 ) {}
	XAsioBuffer::XAsioBuffer( size_t size ) : m_bufData( malloc( size ), size, 0, true ), m_iCursorPos( 0 ) {}
	XAsioBuffer::~XAsioBuffer()
	{
		m_bufData.release();
	}

	void* XAsioBuffer::getData() { return m_bufData._pData; }
	const void*	XAsioBuffer::getData() const { return m_bufData._pData; }

	size_t XAsioBuffer::getAllocatedSize() const { return m_bufData._allocatedSize; }
	size_t XAsioBuffer::getDataSize() const { return m_bufData._dataSize; }
	void XAsioBuffer::setDataSize( size_t size ) { m_bufData._dataSize = size; }
	
	void XAsioBuffer::detach() { m_bufData._bOwnsData = false; }
	void XAsioBuffer::attach() { m_bufData._bOwnsData = true; }

	_inline size_t XAsioBuffer::getRemainSize() { return m_bufData._pData != NULL ? m_bufData._dataSize - m_iCursorPos : 0; }

	bool XAsioBuffer::resize( size_t newSize )
	{
		if ( ( !m_bufData._bOwnsData && m_bufData._pData != NULL )
			|| m_bufData._allocatedSize >= m_bufData._dataSize + newSize
			|| m_bufData._dataSize + newSize > MAX_PACKET_SIZE )
		{
			return false;
		}
		if ( m_bufData._pData != NULL )
		{
			m_bufData._pData			= realloc( m_bufData._pData, newSize );
			m_bufData._allocatedSize	= newSize;
		}
		else
		{
			m_bufData._pData			= malloc( newSize );
			m_bufData._dataSize			= 0;//newSize;
			m_bufData._allocatedSize	= newSize;
			m_bufData._bOwnsData		= true;
		}
		return true;
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

	void XAsioBuffer::copy( const void* pData, size_t size )
	{
		writeData( (const char*)pData, size );
	}

	void XAsioBuffer::copy( XAsioBuffer& buffer )
	{
		writeBuffer( buffer );
	}

	void XAsioBuffer::writeBuffer( XAsioBuffer& buffer )
	{
		if ( resize( m_bufData._dataSize + buffer.getDataSize() ) )
		{
			memcpy_s( (char*)m_bufData._pData + m_bufData._dataSize, m_bufData._allocatedSize, buffer.getData(), buffer.getDataSize() );
			m_bufData._dataSize += buffer.getDataSize();
		}
	}
	void XAsioBuffer::writeString( std::string& str )
	{
		*this << str;
	}
	void XAsioBuffer::writeData( const char* pData, size_t size )
	{
		if ( resize( m_bufData._dataSize + size ) )
		{
			memcpy_s( (char*)m_bufData._pData + m_bufData._dataSize, m_bufData._allocatedSize, pData, size );
			m_bufData._dataSize += size;
		}		
	}

	XAsioBuffer& XAsioBuffer::operator << ( const std::string &str )
	{
		unsigned short size = (unsigned short)str.size();
		if ( resize( m_bufData._dataSize + size + sizeof(unsigned short) ) )
		{
			char* pCur = (char*)m_bufData._pData + m_bufData._dataSize;
			*(unsigned short*)pCur = size;
			m_bufData._dataSize += sizeof(unsigned short);
			pCur += sizeof(unsigned short);

			memcpy_s( pCur, m_bufData._allocatedSize, str.c_str(), size );
			m_bufData._dataSize += size;
		}
		return *this;
	}

	XAsioBuffer& XAsioBuffer::operator << ( const char* pStr )
	{
		unsigned short size = strlen( pStr );
		if ( resize( m_bufData._dataSize + size + sizeof(unsigned short) ) )
		{
			char* pCur = (char*)m_bufData._pData + m_bufData._dataSize;
			*(unsigned short*)pCur = size;
			m_bufData._dataSize += sizeof(unsigned short);
			pCur += sizeof(unsigned short);

			memcpy_s( pCur, m_bufData._allocatedSize, pStr, size );
			m_bufData._dataSize += size;
		}
		return *this;
	}

	void XAsioBuffer::readBuffer( XAsioBuffer& buffer )
	{
		buffer.copy( *this );
	}
	void XAsioBuffer::readString( std::string& str )
	{
		*this >> str;
	}
	void XAsioBuffer::readData( const char* pData, size_t size )
	{
		assert( getRemainSize() >= size );
		if ( getRemainSize() >= size )
		{
			char* pCur = (char*)m_bufData._pData + m_iCursorPos;
			memcpy_s( (char*)pData, size, pCur, size );
			m_iCursorPos += size;
		}		
	}

	XAsioBuffer& XAsioBuffer::operator >> ( std::string &str )
	{
		assert( getRemainSize() >= sizeof(unsigned short) );
		if ( getRemainSize() < sizeof(unsigned short) )
		{
			return *this; 
		}
		unsigned short size = 0;
		*this >> size;

		assert( getRemainSize() >= size && size <= MAX_PACKET_SIZE );
		if ( getRemainSize() < size || size > MAX_PACKET_SIZE )
		{
			return *this;
		}
		char* pCur = (char*)m_bufData._pData + m_iCursorPos;
		str.assign( pCur, size );		
		m_iCursorPos += size;
		return *this;
	}

	XAsioBuffer& XAsioBuffer::operator >> ( const char* pStr )
	{
		assert( getRemainSize() >= sizeof(unsigned short) );
		if ( getRemainSize() < sizeof(unsigned short) )
		{
			return *this; 
		}
		unsigned short size = 0;
		*this >> size;

		assert( getRemainSize() >= size && size <= MAX_PACKET_SIZE );
		if ( getRemainSize() < size || size > MAX_PACKET_SIZE )
		{
			return *this;
		}
		char* pCur = (char*)m_bufData._pData + m_iCursorPos;
		memcpy_s( (char*)pStr, size, pCur, size );
		m_iCursorPos += size;
		return *this;
	}

	//------------------------------------
	
	XAsioPackageHeader::XAsioPackageHeader() : m_dwFlag(0), m_dwSize(0), m_dwToken(0), m_dwType(0)
	{
	}

	void XAsioPackageHeader::setType( unsigned long type )
	{
		m_dwType = type;
	}

	void XAsioPackageHeader::parseFromBuffer( XAsioBuffer& buff )
	{
		memcpy_s( this, XAsioPackageHeader::getHeaderSize(), buff.getData(), buff.getDataSize() );
		if ( m_dwSize > MAX_PACKET_SIZE )
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