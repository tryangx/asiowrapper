#include "asio/XAsioTCP.h"
#include "util/XStringUtil.h"

namespace XGAME
{
#define TIEMR_INTERVAL		50	

	//-----------------------------
	//	TCPÁ¬½Ó
	XAsioTCPSession::XAsioTCPSession( XAsioService& io )
		: XAsioSession( io ), XAsioTimer( m_ioService ),
		m_isSending( false ), m_isSuspendSend( false ), m_isSuspendDispatch( false ),
		m_sendSize( 0 ), m_recvSize( 0 )
	{
		m_socket = TcpSocketPtr( new tcp::socket( m_ioService ) );
	}

	XAsioTCPSession::~XAsioTCPSession()
	{
		close();
	}

	const TcpSocketPtr XAsioTCPSession::getSocket() const { return m_socket; }

	bool XAsioTCPSession::isOpen() { return m_socket && m_socket->is_open(); }

	void XAsioTCPSession::close()
	{
		if ( m_socket && m_socket->is_open() ) 
		{
			boost::system::error_code err;
			m_socket->shutdown( tcp::socket::shutdown_both, err );
			m_socket->close( err );
		}
		for ( int type = SESSION_SEND_BUFFER; type < SESSION_BUFFER_COUNT; type++ )
		{
			mutex::scoped_lock lock( m_mutexs[type] );
			PACKAGE_CONAINER::iterator it;
			for ( it = std::begin( m_buffers[type] ); it != std::end( m_buffers[type] ); it++ )
			{
				XAsioBuffer& buffer = *it;
				buffer.attach();
			}
			m_buffers[type].clear();
		}
	}

	void XAsioTCPSession::read()
	{
		boost::asio::async_read( *m_socket, boost::asio::buffer( m_readBuffer ),
			boost::asio::transfer_at_least( 1 ),
			m_strand.wrap( boost::bind( &XAsioTCPSession::onReadCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}
	void XAsioTCPSession::read( size_t bufferSize )
	{
		if ( bufferSize > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "read size is out of buffer length" );
			return;
		}
		m_socket->async_read_some( boost::asio::buffer( m_readBuffer, bufferSize ),
			m_strand.wrap( boost::bind( &XAsioTCPSession::onReadCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}
	void XAsioTCPSession::write( XAsioBuffer& buffer )
	{
		mutex::scoped_lock lock( m_mutexs[SESSION_SEND_BUFFER] );

		XAsioBuffer newBuffer;
		newBuffer.copy( buffer );
		newBuffer.detach();
		m_buffers[SESSION_SEND_BUFFER].push_back( newBuffer );
		lock.unlock();

		doSend();
	}

	bool XAsioTCPSession::onTimer( unsigned int id, const void* pUserData )
	{
		switch( id )
		{
		case SESSION_DISPATCH_TIMERID:
			return doRead();
			break;
		default:
			return XAsioTimer::onTimer( id, pUserData );
			break;
		}
		return false;
	}

	size_t XAsioTCPSession::getSendSize() const
	{
		return m_sendSize;
	}
	size_t XAsioTCPSession::getRecvSize() const
	{
		return m_recvSize;
	}

	void XAsioTCPSession::suspendSend( bool b )
	{
		m_isSuspendSend = b;
		if ( !m_isSuspendSend )
		{
			doSend();
		}
	}

	void XAsioTCPSession::suspendDispatch( bool b )
	{
		m_isSuspendDispatch = b;
		if ( !m_isSuspendDispatch )
		{
			setTimer( SESSION_DISPATCH_TIMERID, TIEMR_INTERVAL, NULL );
		}
	}

	void XAsioTCPSession::sendDirectly( const XAsioBuffer& buffer )
	{
		m_isSending = true;

		size_t size = buffer.getDataSize();
		memcpy_s( (void*)m_sendBuffer, MAX_PACKET_SIZE, buffer.getData(), size );
		boost::asio::async_write( *m_socket, boost::asio::buffer( m_sendBuffer, size ),
			m_strand.wrap( boost::bind( &XAsioTCPSession::onWriteCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}

	bool XAsioTCPSession::doSend()
	{
		if ( m_isSending || m_isSuspendSend )
		{
			return false;
		}
		mutex::scoped_lock lock( m_mutexs[SESSION_SEND_BUFFER] );
		if ( !m_buffers[SESSION_SEND_BUFFER].empty() )
		{
			XAsioBuffer& buffer = m_buffers[SESSION_SEND_BUFFER].front();
			sendDirectly( buffer );
			buffer.attach();
			m_buffers[SESSION_SEND_BUFFER].pop_front();
		}
		return !m_buffers[SESSION_SEND_BUFFER].empty();
	}

	bool XAsioTCPSession::doRead()
	{
		mutex::scoped_lock lock( m_mutexs[SESSION_RECV_BUFFER] );
		if ( m_buffers[SESSION_RECV_BUFFER].empty() )
		{
			return false;
		}
		PACKAGE_CONAINER::iterator it;
		for ( it = std::begin( m_buffers[SESSION_RECV_BUFFER] ); it != std::end( m_buffers[SESSION_RECV_BUFFER] ); it++ )
		{
			XAsioBuffer& tempBuffer = *it;
			ON_CALLBACK_PARAM( m_funcReadHandler, tempBuffer );
			tempBuffer.attach();
		}
		m_buffers[SESSION_RECV_BUFFER].clear();
		return true;
	}

	void XAsioTCPSession::onReadCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err ) 
		{
			ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "code:%d err:%s", err.value(), err.message().c_str() ) );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_sessionId );
		}
		else
		{
			m_recvSize += bytesTransferred;

			XAsioBuffer buffer;
			buffer.writeData( m_readBuffer, bytesTransferred );
			if ( m_isSuspendDispatch )
			{
				buffer.detach();
				mutex::scoped_lock lock( m_mutexs[SESSION_RECV_BUFFER] );
				m_buffers[SESSION_RECV_BUFFER].push_back( buffer );
				lock.unlock();
			}
			else
			{
				ON_CALLBACK_PARAM( m_funcReadHandler, buffer );
			}	
		}
	}

	void XAsioTCPSession::onWriteCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err )
		{
			ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "code:%d err:%s", err.value(), err.message().c_str() ) );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_sessionId );
		}	
		else
		{
			m_sendSize += bytesTransferred;

			ON_CALLBACK_PARAM( m_funcWriteHandler, bytesTransferred );						

			m_isSending = false;
			doSend();
		}
	}

	void XAsioTCPSession::onCloseCallback( const boost::system::error_code& err )
	{
		if ( err )
		{
			ON_CALLBACK_PARAM( m_funcLogHandler, outputString( "code:%d err:%s", err.value(), err.message().c_str() ) );
		}
		ON_CALLBACK_PARAM( m_funcCloseHandler, getSessionId() );
	}
}