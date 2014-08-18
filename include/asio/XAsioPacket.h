#pragma once

#include "XAsioBase.h"

#pragma pack( 1 )

namespace XGAME
{
	//消息最大长度
#define MAX_PACKET_SIZE			4096
	
	//------------------------------
	// 临时缓存	
	class XAsioBuffer 
	{
	private:
		struct stBuffInfo
		{
			stBuffInfo( void* pData, size_t allocateSize, size_t dataSize, bool bOwnsData );
			~stBuffInfo();

			void*		_pData;
			size_t		_allocatedSize;
			size_t		_dataSize;
			bool		_bOwnsData;

			void		release();
		};

	public:
		XAsioBuffer();
		XAsioBuffer( const XAsioBuffer& buffer );
		XAsioBuffer( void* pBuffer, size_t size );
		XAsioBuffer( size_t size );
		~XAsioBuffer();
				
		/**
		 * 获取数据所有权
		 */
		void		attach();
		/**
		 * 释放数据所有权
		 */
		void		detach();

		/**
		 * 获取数据
		 */
		void*		getData();
		const void*	getData() const;

		/**
		 * 获取分配空间大小
		 */
		size_t		getAllocatedSize() const;
		/**
		 * 获取数据大小
		 */
		size_t		getDataSize() const;
		/**
		 * 设置数据大小
		 */
		void		setDataSize( size_t size );
		/**
		 * 重新分配空间
		 * @return	返回分配是否成功
		 */
		bool		resize( size_t newSize );

		/**
		 * 浅表复制，用于空数据情况下复制其它来源的数据
		 * 不拥有具备所有权（即需要手动删除）
		 */
		void		clone( void* pData, size_t size );

		/**
		 * (深度)复制全部数据
		 * 拥有所有权（不需要手动删除）
		 */
		void		copy( const void* pData, size_t size );

		/**
		 * (深度)复制全部缓存
		 * 拥有所有权（不需要手动删除）
		 */
		void		copy( XAsioBuffer& buffer );
				
		/**
		 * 追加数据
		 */
		void		writeBuffer( XAsioBuffer& buffer );		
		void		writeString( std::string& str );
		void		writeData( const char* pData, size_t size );

		/**
		 * 序列化输入数据
		 */		
		XAsioBuffer& operator << ( const std::string &str );
		XAsioBuffer& operator << ( const char* pStr );

		template<typename TYPE>
		XAsioBuffer& operator << ( const TYPE &value );
		
		/**
		 * 输出数据
		 */
		void		readBuffer( XAsioBuffer& buffer );		
		void		readString( std::string& str );
		void		readData( const char* pData, size_t size );

		//序列化输出
		XAsioBuffer& operator >> ( std::string &str );
		XAsioBuffer& operator >> ( const char* pStr );

		template<typename TYPE>
		XAsioBuffer& operator >> ( TYPE &value );
	
	private:
		size_t getRemainSize();

	private:
		stBuffInfo	m_bufData;
		size_t		m_iCursorPos;
	};

	template<typename TYPE>
	XAsioBuffer& XAsioBuffer::operator << ( const TYPE &value )
	{
		if ( m_bufData._dataSize + sizeof(TYPE) > MAX_PACKET_SIZE )
		{
			return *this;
		}
		resize( m_bufData._dataSize + sizeof(TYPE) );
		try
		{
			char* pCur = (char*)m_bufData._pData + m_bufData._dataSize;
			*(TYPE*)pCur = value;
		}
		catch(...) {}
		m_bufData._dataSize += sizeof(TYPE);
		return *this;
	}
	template<typename TYPE>
	XAsioBuffer& XAsioBuffer::operator >> ( TYPE &value )
	{
		assert( getRemainSize() >= sizeof(TYPE) );
		if ( getRemainSize() >= sizeof(TYPE) )
		{
			char* pCur = (char*)m_bufData._pData + m_iCursorPos;
			value = *(TYPE*)pCur;
			m_iCursorPos += sizeof(TYPE);
		}
		return *this;
	}

	//-------------------------------------------
	//  包头
	class XAsioPackageHeader
	{
	public:
		inline static size_t	getHeaderSize() { return sizeof(XAsioPackageHeader); }

		enum enHeaderFlag
		{
			EN_HDF_ENCRYPT		=	0x1000,
		};

		XAsioPackageHeader();

		/**
		 * 是否拥有标志
		 */
		bool			hasFlag( int flaginx );
		void			setFlag( int flagInx );

		/**
		 * 设置消息类型
		 */
		void			setType( unsigned long type );

		void			parseFromBuffer( XAsioBuffer& buff );

	public:
		//标志
		unsigned long	m_dwFlag;
		//包长度
		unsigned short	m_dwSize;
		//校验
		unsigned long	m_dwToken;
		//包类型
		unsigned long	m_dwType;
	};

	//------------------------------
	// 包
	class XAsioPackage
	{
	public:
		static size_t	getSize() { return sizeof( XAsioPackage ); }

	public:
		XAsioPackage();

		virtual bool	empty();

		virtual void	reset();

		void			parseFromBuffer( XAsioBuffer& buff );
		
	public:
		int				i;
		char			info[32];
	};

	//------------------------------
	// 用于发送的包
	class XAsioSendPackage : public XAsioPackage
	{
	public:
		XAsioSendPackage( unsigned long type, char* pBuf = NULL )
		{
			m_pMsgBuf = pBuf ? pBuf : m_tempBuffer;
			m_pHeader = (XAsioPackageHeader*)m_pMsgBuf;

			m_pHeader->setType( type );
			m_pHeader->m_dwSize = XAsioPackageHeader::getHeaderSize();
		}

		_inline int		getCurPos() { return m_pHeader->m_dwSize; }

		_inline char*	getCurPtr() { return &m_pMsgBuf[getCurPos()]; };

		void	set( char* pBuf )
		{ 
			m_pMsgBuf = pBuf ? pBuf : m_pMsgBuf;
			m_pHeader = (XAsioPackageHeader*)m_pMsgBuf;
		};

		void	reset()
		{
			m_pHeader->m_dwSize = XAsioPackageHeader::getHeaderSize();
			m_pHeader->setType( m_pHeader->m_dwType );
		}

		template <typename T>
		XAsioSendPackage& operator << ( const T &value )
		{
			assert( getCurPos() + sizeof(T) <= MAX_PACKET_SIZE );
			if ( getCurPos() + sizeof(T) <= MAX_PACKET_SIZE )
			{
				*(T*)(&((char*)m_pHeader)[getCurPos()]) = value;
				m_pHeader->m_dwSize += sizeof(T);
			}
			return *this;
		}

		XAsioSendPackage& operator << ( const std::string& str )
		{
			unsigned short size = (unsigned short)str.size();
			if ( size > MAX_PACKET_SIZE )
			{
				return *this;
			}
			assert( getCurPos() + sizeof(unsigned short) + size <= MAX_PACKET_SIZE );
			if ( getCurPos() + sizeof(unsigned short) + size <= MAX_PACKET_SIZE )
			{	
				*this << size;
				memcpy_s( &((char*)m_pHeader)[getCurPos()], MAX_PACKET_SIZE, str.c_str(), size );
				m_pHeader->m_dwSize += size;
			}
			return *this;
		}

	private:
		char					m_tempBuffer[MAX_PACKET_SIZE];
		char*					m_pMsgBuf;
		XAsioPackageHeader*		m_pHeader;
	};
	
	//------------------------------
	// 用于接收的包
	class XAsioRecvPackage : public XAsioPackage
	{
	public:
		XAsioRecvPackage( const char* pBuf )
		{ 
			reset( pBuf );
		}
				
		void	rollback()
		{
			m_pCurPtr = (char*)m_pHeader + XAsioPackageHeader::getHeaderSize();
		}
		
		void	reset( const char* pBuf )
		{
			assert( pBuf != NULL );
			m_pHeader = (XAsioPackageHeader*)pBuf;
			m_pCurPtr = pBuf + XAsioPackageHeader::getHeaderSize();
		}

		//一般类型的输出
		template <typename T>
		XAsioRecvPackage& operator >> ( T &value )
		{
			assert( getRemainSize() >= sizeof(T) );
			if ( getRemainSize() >= sizeof(T) )
			{
				value = *(T*)m_pCurPtr;
				m_pCurPtr += sizeof(T);
			}
			return *this;
		}

		XAsioRecvPackage& operator >> ( std::string& str )
		{
			assert( getRemainSize() >= sizeof(unsigned short) );
			
			if( getRemainSize() < sizeof(unsigned short) )
			{
				return *this; 
			}
			unsigned short size;
			*this >> size;

			assert( getRemainSize() >= size && size < MAX_PACKET_SIZE );
			if( getRemainSize() < size || size > MAX_PACKET_SIZE )
			{
				return *this;
			}
			char temp[MAX_PACKET_SIZE + 1];
			memcpy_s( temp, MAX_PACKET_SIZE + 1, m_pCurPtr, size );
			temp[size] = '\0';
			str = temp;
			m_pCurPtr += size;
			return *this;
		}

	private:
		unsigned long getRemainSize() { return m_pHeader->m_dwSize - ( m_pCurPtr - ((char*)m_pHeader) ); }

	private:
		const char*				m_pCurPtr;
		XAsioPackageHeader*		m_pHeader;
	};
}