#pragma once

#include "XAsioBase.h"

namespace XASIO
{
	//��Ϣ��󳤶�
#define MAX_PACKAGE_LEN			4096
	
	//------------------------------
	// ��ʱ����	
	class XAsioBuffer 
	{
	private:
		struct stBuffInfo
		{
			stBuffInfo( void* pData, size_t size, bool bOwnsData );
			~stBuffInfo();

			void*		_pData;
			size_t		_allocatedSize;
			size_t		_dataSize;
			bool		_bOwnsData;

			void		release();
		};

	public:
		XAsioBuffer();
		XAsioBuffer( void* pBuffer, size_t size );
		XAsioBuffer( size_t size );
		~XAsioBuffer();
		
		void*		getData();
		const void*	getData() const;

		size_t		getAllocatedSize() const;
		size_t		getDataSize() const;
		void		setDataSize( size_t size );
				
		void		resize( size_t newSize );

		/**
		 * ��������
		 */
		void		copy( const void* pData, size_t size );

		/**
		 * ���ƻ���
		 */
		void		copy( XAsioBuffer& buffer );

		/**
		 * ��ȡ��������Ȩ
		 */
		void		attach();
		/**
		 * �ͷ���������Ȩ
		 */
		void		detach();

	private:
		stBuffInfo	m_bufData;
	};

	std::string	bufferToString( const XAsioBuffer& buffer );
	XAsioBuffer	stringToBuffer( std::string& value );

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

		void			parseFromBuffer( XAsioBuffer& buff );

		void			setType( unsigned long type );

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
		int					i;
		char				info[32];
		XAsioPackageHeader*	m_pHeader;
	};

	//------------------------------
	// ���ڷ��͵İ�
	class XAsioSendPackage : public XAsioPackage
	{
	public:
		XAsioSendPackage( unsigned long type, char* pBuf = NULL )
		{
			set( type, pBuf );
		};

		template <typename T>
		XAsioSendPackage& operator << ( const T &value )
		{
			assert( m_pHeader->m_dwSize + sizeof(T) <= MAX_PACKAGE_LEN );
			if ( m_pHeader->m_dwSize + sizeof(T) <= MAX_PACKAGE_LEN )
			{
				*(T*)(&((char*)m_pHeader)[m_pHeader->m_dwSize]) = value;
				m_pHeader->m_dwSize += sizeof(T);
			}
			return *this;
		};

		XAsioSendPackage& operator << ( const std::string& str )
		{
			unsigned short size = (unsigned short)str.size();
			if ( size > MAX_PACKAGE_LEN )
			{
				assert( 0 );
				return *this;
			}
			assert( m_pHeader->m_dwSize + sizeof(unsigned short) + size <= MAX_PACKAGE_LEN );
			if ( m_pHeader->m_dwSize + sizeof(unsigned short) + size <= MAX_PACKAGE_LEN )
			{	
				*this << size;
				memcpy( &((char*)m_pHeader)[m_pHeader->m_dwSize],str.c_str(), size );
				m_pHeader->m_dwSize += size;
			}
			return *this;
		};

		void	set( char* pBuf )
		{ 
			if ( pBuf != NULL )
			{
				m_pMsgBuf = pBuf;
			}
			m_pHeader = (XAsioPackageHeader*)m_pMsgBuf;
		};

		void	set( unsigned long type, char* pBuf ) 
		{ 
			if ( pBuf == NULL )
			{
				m_pMsgBuf = m_tempBuffer;
			}
			else
			{
				m_pMsgBuf = pBuf;
			}
			m_pHeader = (XAsioPackageHeader*)m_pMsgBuf;
			m_pHeader->setType( type );
			m_pHeader->m_dwSize = XAsioPackageHeader::getHeaderSize();
		};

		void	reset()
		{
			m_pHeader->m_dwSize = XAsioPackageHeader::getHeaderSize();
			m_pHeader->setType( m_pHeader->m_dwType );
		};

		char*	getCurPtr()
		{
			return &m_pMsgBuf[m_pHeader->m_dwSize];
		};

	private:
		char		m_tempBuffer[MAX_PACKAGE_LEN];
		char*		m_pMsgBuf;
	};
	
	//------------------------------
	// ���ڽ��յİ�
	class XAsioRecvPackage : public XAsioPackage
	{
	public:
		XAsioRecvPackage( const char* pBuf )
		{ 
			reset( pBuf );
		};

		unsigned long getRemainSize()
		{
			return m_pHeader->m_dwSize - ( m_pCurPtr - ((char*)m_pHeader) );
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
		};

		XAsioRecvPackage& operator >> ( std::string &str )
		{
			assert( getRemainSize() >= sizeof(unsigned short) );
			
			if( getRemainSize() < sizeof(unsigned short) )
			{
				return *this; 
			}
			unsigned short size;
			*this >> size;

			assert( getRemainSize() >= size && size < MAX_PACKAGE_LEN );
			if( getRemainSize() < size || size > MAX_PACKAGE_LEN )
			{
				return *this;
			}
			char temp[MAX_PACKAGE_LEN + 1];
			memcpy_s( temp, MAX_PACKAGE_LEN + 1, m_pCurPtr, size );
			temp[size] = '\0';
			str = temp;
			m_pCurPtr += size;
			return *this;
		};

	private:
		const char*		m_pCurPtr;
	};
}