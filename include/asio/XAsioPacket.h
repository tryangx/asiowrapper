/**
 * 封包
 *
 * packet由header(包头) + content(包内容)两部分组织
 * header是XAsioPacketHeader
 * content为任意数据，可为空
 *
 * XAsioSendPacket用于发送包，会自动处理包头相关
 * XAsioRecvPacket用于解析包，会自动处理包头相关
 */
#pragma once

#include "XApi.h"
#include <iostream>
#include <sstream>

#pragma pack( push )
#pragma pack( 1 )

namespace XGAME
{
	//消息最大长度
#define MAX_PACKET_SIZE			4096

	enum enXAsioPacketOp
	{
		//C2S/S2C协议
		//用于客服间通信
		EN_POP_MSG,

		//S2S命令
		//用于服务器间转发，包类型等数据不影响，只根据目标ID选择对应的目的服务器
		EN_POP_CMD,

		//应答测试
		EN_POP_ECHO,

		//C2S/S2C心跳
		EN_POP_HEARTBEAT,

		//注册
		EN_POP_REGISTER,
		
		//GM
		EN_POP_GM,
	};

	enum enXAsioPacketHeaderFlag
	{
		//是否可以丢包
		EN_PHFLAG_DISCARD		=	0x100,

		//是否加密
		EN_PHFLAG_ENCRYPT		=	0x1000,

		//XOR
		EN_PHFLAG_ENCRYPT_XOR	=	0x1001,
	};

	//------------------------------
	// 临时缓存	
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
		 * 拷贝构造，不会主动释放
		 */
		XAsioBuffer( const XAsioBuffer& buffer );
		/**
		 * 使用指定长度的数据初始化，不会主动释放
		 */
		XAsioBuffer( void* pBuffer, size_t size );
		/**
		 * 分配初始化长度，会主动释放
		 */
		XAsioBuffer( size_t size );
				
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
		const void*	getData() const;
		//void*		getData();		
		/**
		 * 复制数据
		 * 需要手动free
		 */
		void*		copyData();
		/**
		 * 清除缓存
		 */
		void		clear();

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
		 * 导入数据
		 * 夺取原有缓冲数据的所有权，不复制数据
		 * 不拥有具备所有权（需要手动删除）
		 */
		void		import( void* pData, size_t size );

		/**
		 * 导入数据
		 * 夺取原有缓冲数据的所有权，不复制数据
		 * 不拥有具备所有权（需要手动删除）
		 */
		void		import( XAsioBuffer& buffer );
				
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
	//  包头
	struct XGAME_API XAsioPacketHeader
	{
	public:
		static size_t	getHeaderSize();
		
	public:
		XAsioPacketHeader();

		_inline unsigned short	getCRC()	{ return m_wCRC; }
		_inline unsigned char	getToken()	{ return m_cToken; }

		_inline unsigned char	getOp()		{ return m_cOp; }
		_inline void			setOp( unsigned char op )	{ m_cOp = op; }

		_inline unsigned int	getDestID()	{ return m_dwDestId; }
		_inline void			setDestID( unsigned int id )	{ m_dwDestId = id; }
		
		void	setCmdOp( unsigned int destId );

		/**
		 * 设置消息类型
		 */
		_inline void	setType( unsigned int type ) { m_dwType = type; }
		_inline unsigned int	getType()	{ return m_dwType; }

		/**
		 * 是否拥有标志
		 */
		bool			hasFlag( int flaginx );
		/**
		 * 设置标志
		 */
		void			setFlag( int flagInx );

		/**
		 * 从缓冲类中解释数据
		 */
		//void			input( XAsioBuffer& buff );

	public:
		/**
		 * 校验码
		 */
		unsigned short	m_wCRC;
		/**
		 * 口令
		 */
		unsigned char	m_cToken;
		/**
		 * 操作类型(协议，命令，心跳...)
		 */
		unsigned char	m_cOp;
		/**
		 * 标志位
		 * 是否使用加密,加密算法位,校验算法位etc
		 */
		unsigned short	m_dwFlag;
		/**
		 * 包内容长度
		 */
		unsigned short	m_dwSize;
		/**
		 * 包类型
		 */
		unsigned int	m_dwType;		
		/**
		 * 目标ID
		 */
		unsigned int	m_dwDestId;
	};
	
	//------------------------------
	// 发送的包工具类
	class XGAME_API XAsioSendPacket
	{
	public:
		XAsioSendPacket( unsigned int type, char* pBuf = NULL );

		/**
		 * 得到包头数据
		 */
		XAsioPacketHeader*	getHeader();

		/**
		 * 得到当前位置
		 */
		int		getCurPos();

		/**
		 * 得到当前数据指针位置
		 */
		char*	getCurPtr();
				
		/**
		 * 重置包
		 */
		void	reset();

		/**
		 * 输出缓冲数据，用于发送
		 */
		void	output( XAsioBuffer& buff );

		/**
		 * 序列化输入，生成包
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
	//	接收包的工具类
	class XGAME_API XAsioRecvPacket
	{
	public:
		XAsioRecvPacket();
		~XAsioRecvPacket();

		/**
		 * 拷贝构造
		 */
		XAsioRecvPacket( const XAsioRecvPacket& packet );

		/**
		 * 来源ID
		 */
		void				setFromId( unsigned int id );
		unsigned int		getFromID();

		/**
		 * 来源ID
		 */
		void		clear();
				
		/**
		 * 得到包头数据
		 * 返回可能为空
		 */
		XAsioPacketHeader*	getHeader();

		/**
		 * 包是否为空（需要从包头开始获取）
		 */
		bool		isEmpty();

		/**
		 * 包是否可以解析
		 */
		bool		isReady();
				
		/**
		 * 重置解析位置
		 */
		void		reset();

		/**
		 * 获取数据所有权
		 */
		void		attach();
		/**
		 * 释放数据所有权
		 */
		void		detach();

		/**
		 * 克隆数据
		 * 从对象包复制一份到本地
		 */
		void		clone( XAsioRecvPacket& packet );

		/**
		 * 克隆数据
		 * 从对象包复制一份到本地
		 */
		void		import( XAsioRecvPacket& packet );

		/**
		 * 导入包流数据
		 * 复制指定缓冲数据到本地
		 */
		void		input( XAsioBuffer& buff );
				
		/**
		 * 导入包流数据
		 * 夺取原有缓冲数据的所有权，不复制数据
		 */
		void		import( XAsioBuffer& buff );

		/**
		 * 导出数据
		 */
		void		exportBuffer( XAsioBuffer& buff );
		
		/**
		 * 序列化输出，解析包
		 */
		XAsioRecvPacket& operator >> ( std::string& str );
		XAsioRecvPacket& operator >> ( char* pStr );

		/**
		 * 序列化输出，解析包
		 */
		template <typename TYPE>
		XAsioRecvPacket& operator >> ( TYPE &value );

	private:
		unsigned int getRemainSize();

		void	setData( const char* pBuffer, size_t size );
		
		void	setHeader( const char* pBuffer, size_t size );

		void	setPacket( const char* pBuffer, size_t size );

	private:
		XAsioBuffer				m_headerBuff;
		XAsioBuffer				m_packetBuff;
		bool					m_bHeaderReaded;

		unsigned int			m_iFromId;

		XAsioPacketHeader*		m_pHeader;
		char*					m_pData;
		char*					m_pCurPtr;
	};

#pragma pack( pop )

#include "XAsioPacket.hpp"
}

