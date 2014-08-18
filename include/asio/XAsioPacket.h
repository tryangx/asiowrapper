#pragma once

#include "XAsioBase.h"

#pragma pack( 1 )

namespace XGAME
{
	//��Ϣ��󳤶�
#define MAX_PACKET_SIZE			4096
	
	//------------------------------
	// ��ʱ����	
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
		 * ��ȡ��������Ȩ
		 */
		void		attach();
		/**
		 * �ͷ���������Ȩ
		 */
		void		detach();

		/**
		 * ��ȡ����
		 */
		void*		getData();
		const void*	getData() const;

		/**
		 * ��ȡ����ռ��С
		 */
		size_t		getAllocatedSize() const;
		/**
		 * ��ȡ���ݴ�С
		 */
		size_t		getDataSize() const;
		/**
		 * �������ݴ�С
		 */
		void		setDataSize( size_t size );
		/**
		 * ���·���ռ�
		 * @return	���ط����Ƿ�ɹ�
		 */
		bool		resize( size_t newSize );

		/**
		 * ǳ���ƣ����ڿ���������¸���������Դ������
		 * ��ӵ�о߱�����Ȩ������Ҫ�ֶ�ɾ����
		 */
		void		clone( void* pData, size_t size );

		/**
		 * (���)����ȫ������
		 * ӵ������Ȩ������Ҫ�ֶ�ɾ����
		 */
		void		copy( const void* pData, size_t size );

		/**
		 * (���)����ȫ������
		 * ӵ������Ȩ������Ҫ�ֶ�ɾ����
		 */
		void		copy( XAsioBuffer& buffer );
				
		/**
		 * ׷������
		 */
		void		writeBuffer( XAsioBuffer& buffer );		
		void		writeString( std::string& str );
		void		writeData( const char* pData, size_t size );

		/**
		 * ���л���������
		 */		
		XAsioBuffer& operator << ( const std::string &str );
		XAsioBuffer& operator << ( const char* pStr );

		template<typename TYPE>
		XAsioBuffer& operator << ( const TYPE &value );
		
		/**
		 * �������
		 */
		void		readBuffer( XAsioBuffer& buffer );		
		void		readString( std::string& str );
		void		readData( const char* pData, size_t size );

		//���л����
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
	//  ��ͷ
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
		 * �Ƿ�ӵ�б�־
		 */
		bool			hasFlag( int flaginx );
		void			setFlag( int flagInx );

		/**
		 * ������Ϣ����
		 */
		void			setType( unsigned long type );

		void			parseFromBuffer( XAsioBuffer& buff );

	public:
		//��־
		unsigned long	m_dwFlag;
		//������
		unsigned short	m_dwSize;
		//У��
		unsigned long	m_dwToken;
		//������
		unsigned long	m_dwType;
	};

	//------------------------------
	// ��
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
	// ���ڷ��͵İ�
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
	// ���ڽ��յİ�
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

		//һ�����͵����
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