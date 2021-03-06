#include "asio/XAsioPacket.h"
#include <assert.h>

namespace XGAME
{
	//------------------------------
	// ��ʱ����

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
		_pData			= NULL;
		_bOwnsData		= false;
	}
		
	XAsioBuffer::XAsioBuffer() : m_bufData( NULL, 0, 0, false ), m_iCursorPos( 0 ) {}
	XAsioBuffer::XAsioBuffer( const XAsioBuffer& buffer ) : m_bufData( buffer.m_bufData._pData, buffer.m_bufData._allocatedSize, buffer.m_bufData._dataSize, false ), m_iCursorPos( 0 )
	{}
	XAsioBuffer::XAsioBuffer( void* pBuffer, size_t size ) : m_bufData( pBuffer, size, size, false ), m_iCursorPos( 0 )
	{}
	XAsioBuffer::XAsioBuffer( size_t size ) : m_bufData( malloc( size ), size, 0, true ), m_iCursorPos( 0 )
	{}
	XAsioBuffer::~XAsioBuffer()
	{
		m_bufData.release();
	}
	
	const void*	XAsioBuffer::getData() const { return m_bufData._pData; }
	//void* XAsioBuffer::getData() { return m_bufData._pData; }

	void* XAsioBuffer::copyData()
	{
		void* pData = malloc( m_bufData._dataSize );
		memcpy_s( pData, m_bufData._dataSize, m_bufData._pData, m_bufData._dataSize );
		return pData;
	}

	void XAsioBuffer::clear() { m_bufData.release(); }

	size_t XAsioBuffer::getAllocatedSize() const { return m_bufData._allocatedSize; }
	size_t XAsioBuffer::getDataSize() const { return m_bufData._dataSize; }
	void XAsioBuffer::setDataSize( size_t size ) { m_bufData._dataSize = size; }
	
	void XAsioBuffer::detach()
	{
		m_bufData._bOwnsData = false;
	}
	void XAsioBuffer::attach()
	{
		m_bufData._bOwnsData = true;
	}

	_inline size_t XAsioBuffer::getRemainSize() { return m_bufData._pData != NULL ? m_bufData._dataSize - m_iCursorPos : 0; }

	bool XAsioBuffer::resize( size_t newSize )
	{
		if ( !m_bufData._bOwnsData && m_bufData._pData != NULL )
		{
			throw std::runtime_error( "Buffer is invalid, cann't resize!" );
			return false;
		}
		if ( m_bufData._allocatedSize >= m_bufData._dataSize + newSize || m_bufData._dataSize + newSize > MAX_PACKET_SIZE )
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
	
	void XAsioBuffer::import( void* pData, size_t size )
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
	void XAsioBuffer::import( XAsioBuffer& buffer )
	{
		if ( m_bufData._pData && m_bufData._bOwnsData )
		{
			throw std::runtime_error( "data is exist" );
			return;
		}
		m_bufData._pData			= buffer.m_bufData._pData;
		m_bufData._allocatedSize	= buffer.m_bufData._allocatedSize;
		m_bufData._dataSize			= buffer.m_bufData._dataSize;
		m_bufData._bOwnsData		= buffer.m_bufData._bOwnsData;

		buffer.detach();
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
		writeData( (const char*)buffer.getData(), buffer.getDataSize() );
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
		if ( getRemainSize() < size )
		{
			throw std::runtime_error( "out of packet size" );
			return;
		}
		char* pCur = (char*)m_bufData._pData + m_iCursorPos;
		memcpy_s( (char*)pData, size, pCur, size );
		m_iCursorPos += size;
	}

	XAsioBuffer& XAsioBuffer::operator >> ( std::string &str )
	{
		if ( getRemainSize() < sizeof(unsigned short) )
		{
			throw std::runtime_error( "out of packet size" );
			return *this; 
		}
		unsigned short size = 0;
		*this >> size;

		if ( getRemainSize() < size || size > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "out of packet size" );
			return *this;
		}
		char* pCur = (char*)m_bufData._pData + m_iCursorPos;
		str.assign( pCur, size );		
		m_iCursorPos += size;
		return *this;
	}

	XAsioBuffer& XAsioBuffer::operator >> ( const char* pStr )
	{
		if ( getRemainSize() < sizeof(unsigned short) )
		{
			throw std::runtime_error( "out of packet size" );
			return *this; 
		}
		unsigned short size = 0;
		*this >> size;

		if ( getRemainSize() < size || size > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "out of packet size" );
			return *this;
		}
		char* pCur = (char*)m_bufData._pData + m_iCursorPos;
		memcpy_s( (char*)pStr, size, pCur, size );
		m_iCursorPos += size;
		return *this;
	}

	//------------------------------------

	size_t XAsioPacketHeader::getHeaderSize()
	{
		return sizeof(XAsioPacketHeader);
	}
	
	XAsioPacketHeader::XAsioPacketHeader() : 
		m_dwFlag(0), m_dwSize(0), m_wCRC(0), m_cToken(0), m_dwType(0),
		m_cOp( EN_POP_MSG ), m_dwDestId(0)
	{
	}

	void XAsioPacketHeader::setCmdOp( unsigned int destId )
	{
		m_cOp = EN_POP_CMD;
		m_dwDestId = destId;
	}
	
	//------------------------------------

	XAsioSendPacket::XAsioSendPacket( unsigned int type, char* pBuf )
	{
#ifdef _DEBUG
		memset( m_tempBuffer, 0, sizeof( m_tempBuffer ) );
#endif
		m_pData = pBuf ? pBuf : m_tempBuffer;		
		m_pHeader = (XAsioPacketHeader*)m_pData;
		m_pHeader->setType( type );
		m_pHeader->m_dwSize = 0;
		m_pCurPtr = m_pData + sizeof(XAsioPacketHeader);
	}

	XAsioPacketHeader*	XAsioSendPacket::getHeader() { return m_pHeader; }

	int	XAsioSendPacket::getCurPos() { return m_pHeader->m_dwSize; }

	char* XAsioSendPacket::getCurPtr() { return m_pCurPtr; };
	
	void XAsioSendPacket::reset()
	{
		m_pHeader->m_dwSize = 0;
		m_pHeader->setType( m_pHeader->m_dwType );
		m_pCurPtr = m_pData + sizeof(XAsioPacketHeader);
	}

	void XAsioSendPacket::output( XAsioBuffer& buff )
	{
		buff.writeData( m_tempBuffer, getCurPos() + sizeof(XAsioPacketHeader) );
	}

	XAsioSendPacket& XAsioSendPacket::operator << ( const char* pStr )
	{
		unsigned short size = strlen( pStr );
		assert( getCurPos() + size <= MAX_PACKET_SIZE );
		if ( getCurPos() + sizeof(unsigned short) + size > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "out of packet size" );
			return *this;
		}
		*this << size;
		memcpy_s( m_pCurPtr, MAX_PACKET_SIZE, pStr, size );
		m_pHeader->m_dwSize += size;
		m_pCurPtr += size;
		return *this;
	}

	XAsioSendPacket& XAsioSendPacket::operator << ( const std::string& str )
	{
		unsigned short size = str.length();
		if ( getCurPos() + sizeof(unsigned short) + size > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "out of packet size" );
			return *this;
		}
		*this << size;
		memcpy_s( m_pCurPtr, MAX_PACKET_SIZE, str.c_str(), size );
		m_pHeader->m_dwSize += size;
		m_pCurPtr += size;
		return *this;
	}

	//------------------------------------

	XAsioRecvPacket::XAsioRecvPacket() : m_pData( NULL ), m_pHeader( NULL ), m_pCurPtr( NULL ), 
		m_bHeaderReaded( false ), m_iFromId( 0 )
	{
	}
	XAsioRecvPacket::XAsioRecvPacket( const XAsioRecvPacket& packet ) : m_iFromId( 0 )
	{
		import( const_cast<XAsioRecvPacket&>( packet ) );
	}
	XAsioRecvPacket::~XAsioRecvPacket()
	{
		clear();
	}
	void XAsioRecvPacket::setFromId( unsigned int id )
	{
		m_iFromId = id;
	}
	unsigned int XAsioRecvPacket::getFromID()
	{
		return m_iFromId;
	}
	bool XAsioRecvPacket::isEmpty()
	{
		return !m_bHeaderReaded;
	}
	bool XAsioRecvPacket::isReady()
	{
		return m_pHeader && !m_bHeaderReaded;
	}
	XAsioPacketHeader* XAsioRecvPacket::getHeader()
	{
		return m_pHeader;
	}
	void XAsioRecvPacket::clear()
	{
		m_iFromId = 0;
		m_headerBuff.clear();
		m_packetBuff.clear();
	}
	void XAsioRecvPacket::reset()
	{
		if ( m_pData )
		{
			m_pCurPtr = m_pData;
		}
	}
	void XAsioRecvPacket::attach()
	{
		m_headerBuff.attach();
		m_packetBuff.attach();
	}
	void XAsioRecvPacket::detach()
	{
		m_headerBuff.detach();
		m_packetBuff.detach();
	}
	void XAsioRecvPacket::clone( XAsioRecvPacket& packet )
	{
		clear();
		m_iFromId = packet.getFromID();
		m_headerBuff.copy( packet.m_headerBuff );
		m_packetBuff.copy( packet.m_packetBuff );
		m_bHeaderReaded = packet.m_bHeaderReaded;
		m_pHeader	= (XAsioPacketHeader*)m_headerBuff.getData();
		m_pData		= (char*)m_packetBuff.getData();
		m_pCurPtr	= m_pData;
	}

	void XAsioRecvPacket::import( XAsioRecvPacket& packet )
	{
		clear();
		m_iFromId = packet.getFromID();
		m_headerBuff.import( packet.m_headerBuff );
		m_packetBuff.import( packet.m_packetBuff );
		m_bHeaderReaded = packet.m_bHeaderReaded;
		m_pHeader	= (XAsioPacketHeader*)m_headerBuff.getData();
		m_pData		= (char*)m_packetBuff.getData();
		m_pCurPtr	= m_pData;
	}

	void XAsioRecvPacket::input( XAsioBuffer& buff )
	{
		setData( (const char*)buff.copyData(), buff.getDataSize() );
	}

	void XAsioRecvPacket::import( XAsioBuffer& buff )
	{
		buff.detach();
		setData( (const char*)buff.getData(), buff.getDataSize() );
	}

	void XAsioRecvPacket::exportBuffer( XAsioBuffer& buff )
	{
		buff.clear();
		buff.writeBuffer( m_headerBuff );
		buff.writeBuffer( m_packetBuff );
	}

	void XAsioRecvPacket::setData( const char* pBuffer, size_t size )
	{
		if ( m_bHeaderReaded )
		{
			setPacket( pBuffer, size );
		}
		else
		{
			setHeader( pBuffer, size );
		}
		m_bHeaderReaded = !m_bHeaderReaded;
		if ( m_pHeader && m_pHeader->m_dwSize == 0 )
		{
			//no content
			m_bHeaderReaded = false;
		}
	}

	void XAsioRecvPacket::setHeader( const char* pBuffer, size_t size )
	{
		m_headerBuff.clear();
		m_headerBuff.import( (void*)pBuffer, size );
		m_headerBuff.attach();
		m_pHeader = (XAsioPacketHeader*)m_headerBuff.getData();
		m_pData		= NULL;
		m_pCurPtr	= NULL;
	}
	void XAsioRecvPacket::setPacket( const char* pBuffer, size_t size )
	{
		m_packetBuff.clear();
		m_packetBuff.import( (void*)pBuffer, size );
		m_packetBuff.attach();
		m_pData		= (char*)m_packetBuff.getData();
		m_pCurPtr	= m_pData;
	}

	XAsioRecvPacket& XAsioRecvPacket::operator >> ( char* pStr )
	{
		if( getRemainSize() < sizeof(unsigned short) )
		{
			throw std::runtime_error( "out of packet size" );
			return *this; 
		}
		unsigned short size;
		*this >> size;

		assert( getRemainSize() >= size && size < MAX_PACKET_SIZE );
		if( getRemainSize() < size || size > MAX_PACKET_SIZE )
		{
			return *this;
		}
		memcpy_s( pStr, size, m_pCurPtr, size );
		m_pCurPtr += size;
		return *this;
	}
	XAsioRecvPacket& XAsioRecvPacket::operator >> ( std::string& str )
	{
		if( getRemainSize() < sizeof(unsigned short) )
		{
			throw std::runtime_error( "out of packet size" );
			return *this;
		}
		unsigned short size;
		*this >> size;

		assert( getRemainSize() >= size && size < MAX_PACKET_SIZE );
		if( getRemainSize() < size || size > MAX_PACKET_SIZE )
		{
			return *this;
		}
		str.assign( m_pCurPtr, size );
		m_pCurPtr += size;
		return *this;
	}
	
	_inline unsigned int XAsioRecvPacket::getRemainSize()
	{
		if ( m_pHeader == NULL || m_pCurPtr == NULL )
		{
			return 0;
		}
		return m_pHeader->m_dwSize - ( m_pCurPtr - m_pData );
	}
}