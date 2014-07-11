#include "../../include/asio/XAsioInterface.h"
#include "../../include/asio/XAsioService.h"

namespace XASIO
{
	//-------------------------------------------
	//	客户端接口实现
	
	XAsioClientInterface::XAsioClientInterface( XAsioService& service )
		: XAsioInterface( service ), m_funcResolveHandler( nullptr )		
	{
	}

	XAsioClientInterface::~XAsioClientInterface()
	{
		m_funcResolveHandler = nullptr;
	}
	
	//-------------------------------------------
	//	服务端接口实现

	XAsioServerInterface::XAsioServerInterface( XAsioService& service )
		: XAsioInterface( service )
	{
	}

	//-------------------------------------------
	//	会话接口实现

	XAsioSessionInterface::XAsioSessionInterface( XAsioService& service )
		: m_service( service ), m_strand( service ), m_bufferSize( 0 ),
		m_funcReadCompleteHandler( nullptr ), m_funcReadHandler( nullptr ), m_funcWriteHandler( nullptr ), m_funcLogHandler( nullptr ), m_funcCloseHandler( nullptr ),
		m_id( 0 ), m_pUserData( nullptr )
	{
	}

	XAsioSessionInterface::~XAsioSessionInterface()
	{
		m_funcReadCompleteHandler	= nullptr;
		m_funcReadHandler			= nullptr;
		m_funcWriteHandler			= nullptr;
		m_funcCloseHandler			= nullptr;
		m_funcLogHandler			= nullptr;

		m_streamRequest.consume( m_streamRequest.size() );
		m_streamResponse.consume( m_streamResponse.size() );
	}

	unsigned int XAsioSessionInterface::getId() const { return m_id; }
	void XAsioSessionInterface::setId( unsigned int id ) { m_id = id; }

	void* XAsioSessionInterface::getUserData() { return m_pUserData; }
	void XAsioSessionInterface::setUserData( void* pData ) { m_pUserData = pData; }

	void XAsioSessionInterface::onRead( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err ) 
		{
			if ( err == boost::asio::error::eof )
			{
				if ( m_funcReadCompleteHandler != nullptr )
				{
					m_funcReadCompleteHandler();
				}
			}
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
			if ( m_funcCloseHandler != nullptr )
			{
				m_funcCloseHandler( m_id );
			}
		}
		else
		{
			if ( m_funcReadHandler != NULL )
			{
				char* pData;
				bool useTempBuffer = true;
#ifdef USE_RECV_BUFFER
				if ( bytesTransferred < MAX_PACKAGE_LEN )
				{
					pData = m_szReadBuffer;
					useTempBuffer = false;
				}
				else
				{
					pData = new char[bytesTransferred + 1];
				}				
#else
				pData = new char[bytesTransferred + 1];
#endif
				pData[bytesTransferred] = 0;
				m_streamResponse.commit( bytesTransferred );
				std::istream stream( &m_streamResponse );
				stream.read( pData, bytesTransferred );
				m_funcReadHandler( XAsioBuffer( pData, bytesTransferred ) );
				if ( useTempBuffer )
				{
					delete [] pData;
				}				
			}
			if ( m_funcReadCompleteHandler != nullptr && m_bufferSize > 0 && bytesTransferred < m_bufferSize )
			{
				m_funcReadCompleteHandler();
			}
		}
		m_streamResponse.consume( m_streamResponse.size() );
	}

	void XAsioSessionInterface::onWrite( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err )
		{
			if ( m_funcLogHandler != nullptr )
			{
				m_funcLogHandler( err.message() );
			}
		}
		else if ( m_funcWriteHandler != nullptr )
		{
			m_funcWriteHandler( bytesTransferred );
		}
	}
}