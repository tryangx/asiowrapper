#include "asio/XAsioTCP.h"
#include "util/XStringUtil.h"

namespace XGAME
{
#define TIEMR_INTERVAL		50	

	//-----------------------------
	//	TCPÁ¬½Ó
	XAsioTCPSession::XAsioTCPSession( XAsioServiceController& controller )
		: XAsioSession( controller ), XAsioTimer( m_ioService ),
		m_isSending( false ), m_isSuspendSend( false ), m_isSuspendDispatch( false ),
		m_dwSendSize( 0 ), m_dwRecvSize( 0 )
	{
		m_socket = TcpSocketPtr( new tcp::socket( m_ioService ) );
	}

	XAsioTCPSession::~XAsioTCPSession()
	{
		close();
	}

	const TcpSocketPtr XAsioTCPSession::getSocket() const
	{
		return m_socket;
	}

	bool XAsioTCPSession::isOpen()
	{
		return m_socket && m_socket->is_open();
	}

	void XAsioTCPSession::close()
	{
		if ( isOpen() ) 
		{
			boost::system::error_code err;
			m_socket->shutdown( tcp::socket::shutdown_both, err );
			m_socket->close( err );
		}
		for ( int type = EN_SESSION_SEND_BUFFER; type < EN_SESSION_BUFFER_COUNT; type++ )
		{
			mutex::scoped_lock lock( m_mutexs[type] );
			PACKET_CONAINER::iterator it;
			for ( it = std::begin( m_packetBuffs[type] ); it != std::end( m_packetBuffs[type] ); it++ )
			{
				XAsioBuffer& buffer = *it;
				buffer.attach();
			}
			m_packetBuffs[type].clear();
		}
	}

	void XAsioTCPSession::recv()
	{
		boost::asio::async_read( *m_socket, boost::asio::buffer( m_recvBuffer ),
			boost::asio::transfer_at_least( 1 ),
			m_strand.wrap( boost::bind( &XAsioTCPSession::onRecvCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}
	void XAsioTCPSession::recv( size_t bufferSize )
	{
		if ( bufferSize > MAX_PACKET_SIZE )
		{
			throw std::runtime_error( "read size is out of buffer length" );
			return;
		}
		m_socket->async_read_some( boost::asio::buffer( m_recvBuffer, bufferSize ),
			m_strand.wrap( boost::bind( &XAsioTCPSession::onRecvCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}
	void XAsioTCPSession::send( XAsioBuffer& buffer )
	{
		mutex::scoped_lock lock( m_mutexs[EN_SESSION_SEND_BUFFER] );

		XAsioBuffer newBuffer;
		newBuffer.copy( buffer );
		newBuffer.detach();
		m_packetBuffs[EN_SESSION_SEND_BUFFER].push_back( newBuffer );
		lock.unlock();

		processSend();
	}

	bool XAsioTCPSession::onTimer( unsigned int id, const void* pUserData )
	{
		switch( id )
		{
		case SESSION_DISPATCH_TIMERID:
			return processRead();
			break;
		default:
			return XAsioTimer::onTimer( id, pUserData );
			break;
		}
		return false;
	}

	size_t XAsioTCPSession::getSendSize() const
	{
		return m_dwSendSize;
	}
	size_t XAsioTCPSession::getRecvSize() const
	{
		return m_dwRecvSize;
	}

	void XAsioTCPSession::suspendSend( bool b )
	{
		m_isSuspendSend = b;
		if ( !m_isSuspendSend )
		{
			processSend();
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

	void XAsioTCPSession::sendImmediately( const XAsioBuffer& buffer )
	{
		m_isSending = true;

		size_t size = buffer.getDataSize();
		memcpy_s( (void*)m_sendBuffer, MAX_PACKET_SIZE, buffer.getData(), size );
		boost::asio::async_write( *m_socket, boost::asio::buffer( m_sendBuffer, size ),
			m_strand.wrap( boost::bind( &XAsioTCPSession::onSendCallback, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) );
	}

	bool XAsioTCPSession::processSend()
	{
		if ( m_isSending || m_isSuspendSend )
		{
			return false;
		}
		mutex::scoped_lock lock( m_mutexs[EN_SESSION_SEND_BUFFER] );
		if ( !m_packetBuffs[EN_SESSION_SEND_BUFFER].empty() )
		{
			XAsioBuffer& buffer = m_packetBuffs[EN_SESSION_SEND_BUFFER].front();
			sendImmediately( buffer );
			buffer.attach();
			m_packetBuffs[EN_SESSION_SEND_BUFFER].pop_front();
		}
		return !m_packetBuffs[EN_SESSION_SEND_BUFFER].empty();
	}

	bool XAsioTCPSession::processRead()
	{
		mutex::scoped_lock lock( m_mutexs[EN_SESSION_RECV_BUFFER] );
		if ( m_packetBuffs[EN_SESSION_RECV_BUFFER].empty() )
		{
			return false;
		}
		PACKET_CONAINER::iterator it;
		for ( it = std::begin( m_packetBuffs[EN_SESSION_RECV_BUFFER] ); it != std::end( m_packetBuffs[EN_SESSION_RECV_BUFFER] ); it++ )
		{
			XAsioBuffer& tempBuffer = *it;
			ON_CALLBACK_PARAM( m_funcRecvHandler, tempBuffer );
			tempBuffer.attach();
		}
		m_packetBuffs[EN_SESSION_RECV_BUFFER].clear();
		return true;
	}

	void XAsioTCPSession::onRecvCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err ) 
		{
			XAsioLog::getInstance()->writeLog( "session:%d,code:%d,err:%s", m_dwSessionId, err.value(), err.message().c_str() );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_dwSessionId );
		}
		else
		{
			m_dwRecvSize += bytesTransferred;

			XAsioBuffer buffer;
			buffer.writeData( m_recvBuffer, bytesTransferred );
			if ( m_isSuspendDispatch )
			{
				buffer.detach();
				mutex::scoped_lock lock( m_mutexs[EN_SESSION_RECV_BUFFER] );
				m_packetBuffs[EN_SESSION_RECV_BUFFER].push_back( buffer );
				lock.unlock();
			}
			else
			{
				ON_CALLBACK_PARAM( m_funcRecvHandler, buffer );
			}	
		}
	}

	void XAsioTCPSession::onSendCallback( const boost::system::error_code& err, size_t bytesTransferred )
	{
		if ( err )
		{
			XAsioLog::getInstance()->writeLog( "session:%d,code:%d,err:%s", m_dwSessionId, err.value(), err.message().c_str() );
			ON_CALLBACK_PARAM( m_funcCloseHandler, m_dwSessionId );
		}	
		else
		{
			m_dwSendSize += bytesTransferred;

			ON_CALLBACK_PARAM( m_funcSendHandler, bytesTransferred );						

			m_isSending = false;
			processSend();
		}
	}

	void XAsioTCPSession::onCloseCallback( const boost::system::error_code& err )
	{
		if ( err )
		{
			XAsioLog::getInstance()->writeLog( "session:%d,code:%d err:%s", m_dwSessionId, err.value(), err.message().c_str() );
		}
		ON_CALLBACK_PARAM( m_funcCloseHandler, getSessionId() );
	}
}