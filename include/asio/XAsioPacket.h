#pragma once

#include "XApi.h"
#include <iostream>
#include <sstream>

#pragma pack( push )
#pragma pack( 1 )

namespace XGAME
{
	//��Ϣ��󳤶�
#define MAX_PACKET_SIZE			4096

	enum enXAsioPacketOp
	{
		EN_POP_MSG,			//between c2s
		EN_POP_CMD,			//between s2s
		EN_POP_HEARTBEAT,	//heart beat
		EN_POP_ECHO,		//echo test
	};

	enum enXAsioPacketHeaderFlag
	{
		EN_PHFLAG_ENCRYPT		=	0x1000,
	};

	//------------------------------
	// ��ʱ����	
	class XGAME_API XAsioBuffer 
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
		~XAsioBuffer();
		/**
		 * �������죬���������ͷ�
		 */
		XAsioBuffer( const XAsioBuffer& buffer );
		/**
		 * ʹ��ָ�����ȵ����ݳ�ʼ�������������ͷ�
		 */
		XAsioBuffer( void* pBuffer, size_t size );
		/**
		 * �����ʼ�����ȣ��������ͷ�
		 */
		XAsioBuffer( size_t size );
				
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
		const void*	getData() const;
		//void*		getData();		
		/**
		 * ��������
		 * ��Ҫ�ֶ�free
		 */
		void*		copyData();
		/**
		 * �������
		 */
		void		clear();

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
		 * ��������
		 * ��ȡԭ�л������ݵ�����Ȩ������������
		 * ��ӵ�о߱�����Ȩ����Ҫ�ֶ�ɾ����
		 */
		void		import( void* pData, size_t size );

		/**
		 * ��������
		 * ��ȡԭ�л������ݵ�����Ȩ������������
		 * ��ӵ�о߱�����Ȩ����Ҫ�ֶ�ɾ����
		 */
		void		import( XAsioBuffer& buffer );
				
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

		XAsioBuffer& operator >> ( std::string &str );
		XAsioBuffer& operator >> ( const char* pStr );
		template<typename TYPE>
		XAsioBuffer& operator >> ( TYPE &value );
	
	private:
		size_t		getRemainSize();

	private:
		stBuffInfo	m_bufData;
		size_t		m_iCursorPos;
	};

	//-------------------------------------------
	//  ��ͷ
	struct XGAME_API XAsioPacketHeader
	{
	public:
		static size_t	getHeaderSize();
		
	public:
		XAsioPacketHeader();

		/**
		 * �Ƿ�ӵ�б�־
		 */
		bool			hasFlag( int flaginx );
		/**
		 * ���ñ�־
		 */
		void			setFlag( int flagInx );

		/**
		 * ������Ϣ����
		 */
		void			setType( unsigned long type );

		/**
		 * �ӻ������н�������
		 */
		void			input( XAsioBuffer& buff );

	public:
		//��־
		unsigned long	m_dwFlag;
		//������
		unsigned short	m_dwSize;
		//У��
		unsigned long	m_dwToken;
		//������ʽ(�����Ϣ������...)
		unsigned char	m_cOp;
		//������
		unsigned long	m_dwType;
	};
	
	//------------------------------
	// ���͵İ�������
	class XGAME_API XAsioSendPacket
	{
	public:
		XAsioSendPacket( unsigned long type, char* pBuf = NULL );

		/**
		 * �õ���ͷ����
		 */
		XAsioPacketHeader*	getHeader();

		/**
		 * �õ���ǰλ��
		 */
		int		getCurPos();

		/**
		 * �õ���ǰ����ָ��λ��
		 */
		char*	getCurPtr();
				
		/**
		 * ���ð�
		 */
		void	reset();

		/**
		 * ����������ݣ����ڷ���
		 */
		void	output( XAsioBuffer& buff );

		/**
		 * ���л����룬���ɰ�
		 */
		XAsioSendPacket& operator << ( const std::string& str );
		XAsioSendPacket& operator << ( const char* pStr );

		template <typename TYPE>
		XAsioSendPacket& operator << ( const TYPE &value );

	private:
		char					m_tempBuffer[MAX_PACKET_SIZE];
		char*					m_pData;
		char*					m_pCurPtr;
		XAsioPacketHeader*		m_pHeader;
	};
	
	//------------------------------
	//	���հ��Ĺ�����
	class XGAME_API XAsioRecvPacket
	{
	public:
		XAsioRecvPacket();
		~XAsioRecvPacket();

		/**
		 * ��������
		 */
		XAsioRecvPacket( const XAsioRecvPacket& packet );
				
		/**
		 * �õ���ͷ����
		 * ���ؿ���Ϊ��
		 */
		XAsioPacketHeader*	getHeader();

		/**
		 * ���Ƿ�Ϊ�գ���Ҫ�Ӱ�ͷ��ʼ��ȡ��
		 */
		bool	isEmpty();

		/**
		 * ���Ƿ���Խ���
		 */
		bool	isReady();
				
		/**
		 * ���ý���λ��
		 */
		void	reset();

		/**
		 * ��ȡ��������Ȩ
		 */
		void		attach();
		/**
		 * �ͷ���������Ȩ
		 */
		void		detach();

		/**
		 * ��¡����
		 * �Ӷ��������һ�ݵ�����
		 */
		void	clone( XAsioRecvPacket& packet );

		/**
		 * ��¡����
		 * �Ӷ��������һ�ݵ�����
		 */
		void	import( XAsioRecvPacket& packet );

		/**
		 * ��������
		 * ����ָ���������ݵ�����
		 */
		void	input( XAsioBuffer& buff );
				
		/**
		 * ��������
		 * ��ȡԭ�л������ݵ�����Ȩ������������
		 */
		void	import( XAsioBuffer& buff );
		
		/**
		 * ���л������������
		 */
		XAsioRecvPacket& operator >> ( std::string& str );
		XAsioRecvPacket& operator >> ( char* pStr );

		/**
		 * ���л������������
		 */
		template <typename TYPE>
		XAsioRecvPacket& operator >> ( TYPE &value );

	private:
		unsigned long getRemainSize();

		void	setData( const char* pBuffer, size_t size );
		
		void	setHeader( const char* pBuffer, size_t size );

		void	setPacket( const char* pBuffer, size_t size );

	private:
		XAsioBuffer				m_headerBuff;
		XAsioBuffer				m_packetBuff;
		bool					m_bHeaderReaded;

		XAsioPacketHeader*		m_pHeader;
		char*					m_pData;
		char*					m_pCurPtr;
	};

#pragma pack( pop )

#include "XAsioPacket.hpp"
}

