#include "../../include/asio/XAsioSession.h"

namespace XASIO
{
	//-------------------------------------------
	//	会话接口实现

	XAsioSession::XAsioSession( XAsioService& service )
		: m_service( service ), m_strand( m_service.getIOService() ),
		m_funcReadHandler( nullptr ), m_funcWriteHandler( nullptr ), m_funcLogHandler( nullptr ), m_funcCloseHandler( nullptr ),
		m_sessionId( 0 ), m_bufferSize( 0 )
	{
	}

	XAsioSession::~XAsioSession()
	{
		release();

		m_streamRequest.consume( m_streamRequest.size() );
		m_streamResponse.consume( m_streamResponse.size() );
	}

	unsigned int XAsioSession::getSessionId() const { return m_sessionId; }
	void XAsioSession::setSessionId( unsigned int id ) { m_sessionId = id; }

	void XAsioSession::release()
	{
		m_funcReadHandler			= nullptr;
		m_funcWriteHandler			= nullptr;
		m_funcCloseHandler			= nullptr;
		m_funcLogHandler			= nullptr;
	}

	void XAsioSession::onReadCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err ) 
		{
			ON_CALLBACK_PARAM( m_funcLogHandler, err.message() );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_sessionId );
		}
		else
		{
			if ( m_funcReadHandler != NULL )
			{
				m_streamResponse.commit( bytesTransferred );
				std::istream stream( &m_streamResponse );
				stream.read( m_readBuffer.data(), bytesTransferred );
				m_funcReadHandler( XAsioBuffer( m_readBuffer.data(), bytesTransferred ) );
			}
		}
		m_streamResponse.consume( m_streamResponse.size() );
	}

	void XAsioSession::onWriteCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err )
		{
			ON_CALLBACK_PARAM( m_funcLogHandler, err.message() );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_sessionId );
		}
		else
		{
			ON_CALLBACK_PARAM( m_funcWriteHandler, bytesTransferred );
		}
	}
}