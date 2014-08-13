#include "../../include/asio/XAsioSession.h"
#include "../../include/util/XStringUtil.h"

namespace XGAME
{
	//-------------------------------------------
	//	会话接口实现

	XAsioSession::XAsioSession( XAsioService& service )
		: m_service( service ), m_ioService( m_service.getIOService() ), m_strand( m_ioService ),
		m_funcReadHandler( nullptr ), m_funcWriteHandler( nullptr ), m_funcLogHandler( nullptr ), m_funcCloseHandler( nullptr ),
		m_sessionId( 0 )
	{
	}

	XAsioSession::~XAsioSession()
	{
		release();
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
			ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "[%d]%s", err.value(), err.message() ) );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_sessionId );
		}
		else
		{
			if ( m_funcReadHandler != NULL )
			{
				XAsioBuffer buffer;
				buffer.copy( m_readBuffer, bytesTransferred );				
				ON_CALLBACK_PARAM( m_funcReadHandler, buffer );
			}
		}
	}

	void XAsioSession::onWriteCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err )
		{
			ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "[%d]%s", err.value(), err.message() ) );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_sessionId );
		}
		else
		{
			ON_CALLBACK_PARAM( m_funcWriteHandler, bytesTransferred );
		}
	}
}