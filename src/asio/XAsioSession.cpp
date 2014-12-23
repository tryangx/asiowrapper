#include "asio/XAsioSession.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//-------------------------------------------
	//	会话接口实现

	XAsioSession::XAsioSession( XAsioServiceController& controller )
		: m_controller( controller ), m_ioService( m_controller.getAsioIOService() ), m_strand( m_ioService ),
		m_funcRecvHandler( nullptr ), m_funcSendHandler( nullptr ), m_funcCloseHandler( nullptr ),
		m_dwSessionId( 0 )
	{
	}

	XAsioSession::~XAsioSession()
	{
		release();
	}

	unsigned long XAsioSession::getSessionId() const
	{
		return m_dwSessionId;
	}
	void XAsioSession::setSessionId( unsigned long id )
	{
		m_dwSessionId = id;
	}

	void XAsioSession::setRecvHandler( std::function<void( XAsioBuffer& )> handler )
	{
		m_funcRecvHandler = handler;
	}
	void XAsioSession::setSendHandler( std::function<void( size_t )> handler )
	{
		m_funcSendHandler = handler;
	}
	void XAsioSession::setCloseHandler( std::function<void( size_t )> handler )
	{
		m_funcCloseHandler = handler;
	}

	void XAsioSession::release()
	{
		m_funcRecvHandler			= nullptr;
		m_funcSendHandler			= nullptr;
		m_funcCloseHandler			= nullptr;
	}

	void XAsioSession::onRecvCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err ) 
		{
			XAsioLog::getInstance()->writeLog( "session:%d,code:%d,err:%s", err.value(), err.message().c_str() );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_dwSessionId );
		}
		else
		{
			if ( m_funcRecvHandler != NULL )
			{
				XAsioBuffer buffer;
				buffer.copy( m_recvBuffer, bytesTransferred );				
				ON_CALLBACK_PARAM( m_funcRecvHandler, buffer );
			}
		}
	}

	void XAsioSession::onSendCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err )
		{
			XAsioLog::getInstance()->writeLog( "session:%d,code:%d,err:%s", m_dwSessionId, err.value(), err.message().c_str() );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_dwSessionId );
		}
		else
		{
			ON_CALLBACK_PARAM( m_funcSendHandler, bytesTransferred );
		}
	}
}