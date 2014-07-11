#pragma once

#include "XAsioBase.h"

namespace XASIO
{
	//------------------------------
	// 临时缓存
	
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
		};

	public:
		XAsioBuffer();
		XAsioBuffer( void* pBuffer, size_t size );
		XAsioBuffer( size_t size );
		
		void*		getData();
		const void*	getData() const;

		size_t		getAllocatedSize() const;
		size_t		getDataSize() const;
		void		setDataSize( size_t size );
				
		void		resize( size_t newSize );

		void		copyFrom( const void* pData, size_t size );

	private:
		boost::shared_ptr<stBuffInfo>		m_bufData;
	};

	std::string	bufferToString( const XAsioBuffer& buffer );
	XAsioBuffer	stringToBuffer( std::string& value );

	class XAsioPackageHeader
	{
	public:
		static size_t	getSize() { return sizeof(XAsioPackageHeader); }

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

		void			parseFromBuffer( XAsioBuffer& buff );

	public:
		//标志
		unsigned long	m_dwFlag;

		//包长度
		unsigned short	m_dwPackageSize;

		//校验
		unsigned long	m_dwToken;
	};

	class XAsioPackage
	{
	public:
		static size_t	getSize() { return sizeof( XAsioPackage ); }

	public:
		XAsioPackage();

		void			parseFromBuffer( XAsioBuffer& buff );

	public:
		int				i;
		char			info[32];
	};

	char*	outputString( const char* pszFormat, ... );
}