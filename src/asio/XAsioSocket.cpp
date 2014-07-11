/*
#include "XAsioSocket.h"

namespace XASIO
{
#define DISPATCH_TIMERID			0
#define SUSPEND_DISPATCH_TIMERID	1
#define POST_TIMERID				2

	template<typename MSGTYPE, typename SOCKET>
	XAsioSocket<typename MSGTYPE, typename SOCKET>::XAsioSocket( io_service& ioService ) : 
	SOCKET( ioService ),
		XAsioTimer( ioService ), 
		packer( boost::make_shared<DEFAULT_PACKER>() )
	{
		resetState();
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::resetState()
	{
		m_bIsPosting = false;
		m_bIsSending = false;
		m_bIsSuspendSendMsg = false;
		m_bIsDispatching = false;
		m_bIsSuspendDispatchMsg = false;
		m_bIsStarted = false;
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::clearBuffers()
	{
		getPostBuffer().clear();
		getSendBuffer().clear();
		getRecvBuffer().clear();
		getTempBuffer().clear();
	}

	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::isStarted() const
	{
		return m_bIsStarted;
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::start()
	{
		mutex::scoped_lock lock(start_mutex);
		if ( !m_bIsStarted )
		{
			m_bIsStarted = doStart();
		}
	}

	//return false if send buffer is empty or sending not allowed or io_service stopped
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::sendMsg()
	{
		mutex::scoped_lock lock(send_msg_buffer_mutex);
		return doSendMsg();
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::suspendSendMsg( bool suspend )
	{
		m_bIsSuspendSendMsg = suspend
		if ( !m_bIsSuspendSendMsg )
		{
			sendMsg();
		}
	}

	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::isSuspendSendMsg() const
	{
		return m_bIsSuspendSendMsg;
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::suspenDispatchMsg( bool suspend )
	{
		m_bIsSuspendDispatchMsg = suspend;
		stopTimer( SUSPEND_DISPATCH_TIMERID );
		doDispatchMsg( true );
	}

	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::isSuspendDispatchMsg() const
	{
		return m_bIsSuspendDispatchMsg;
	}

	//get or change the packer at runtime
	template<typename MSGTYPE, typename SOCKET>
	boost::shared_ptr<XAsioBasePacker> XAsioSocket<typename MSGTYPE, typename SOCKET>::getPacker() const
	{
		return m_packer;
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::setPacker( const boost::shared_ptr<XAsioBasePacker>& packer )
	{
		m_packer = packer;
	}

	//if you use can_overflow = true to invoke send_msg or send_native_msg, it will always succeed
	//no matter whether the send buffer is available
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::isSendBufferAvailable()
	{
		mutex::scoped_lock lock( getSendMutex() );
		return getSendBuffer().size() < MAX_MSG_NUM;
	}

	//don't use the packer but insert into the send_msg_buffer directly
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::sendMsgDirectly( const MSGTYPE& msg, bool isOverflow )
	{
		return sendMsgDirectly( MSGTYPE( msg ), isOverflow );
	}
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::sendMsgDirectly( MSGTYPE&& msg, bool isOverflow )
	{
		mutex::scoped_lock lock( getSendMutex() );
		if ( isOverflow || getSendBuffer().size() < MAX_MSG_NUM )
		{
			return doSendMsgDirectly( std::move( msg ) );
		}
		return false;
	}

	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::postMsgDirectly( const MSGTYPE& msg, bool isOverflow )
	{
		return postMsgDirectly( MsgType(msg), isOverflow );
	}
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::postMsgDirectly( MSGTYPE&& msg, bool isOverflow )
	{
		if ( sendMsgDirectly( std::move( msg ), isOverflow ) )
		{
			return true;
		}
		else
		{
			mutex::scoped_lock lock( getPostMutex() );
			return doPostMsgDirectly( std::move( msg ) );
		}
	}

	//how many msgs waiting for sending(sending_msg = true) or dispatching
	template<typename MSGTYPE, typename SOCKET>
	size_t XAsioSocket<typename MSGTYPE, typename SOCKET>::getPendingMsgNum( EN_BUFFER_TYPE enBufferType )
	{
		mutex::scoped_lock lock( m_msgMutexs[enBufferType] );
		return m_msgBuffers[enBufferType].size();
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::peekFirstPendingMsg( MSGTYPE& msg, EN_BUFFER_TYPE enBufferType )
	{
		msg.clear();
		//msgs in send buffer and post buffer are packed
		//msgs in recv buffer are unpacked
		mutex::scoped_lock lock( m_msgMutex[enBufferType] );
		if ( !m_msgBuffers[enBufferType].empty() )
		{
			msg = m_msgBuffers[enBufferType].front();
		}
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::popFirstPendingMsg( MSGTYPE& msg, EN_BUFFER_TYPE enBufferType )
	{
		msg.clear();
		//msgs in send buffer and post buffer are packed
		//msgs in recv buffer are unpacked
		mutex::scoped_lock lock( m_msgMutex[enBufferType] );
		if ( !m_msgBuffers[enBufferType].empty() )
		{
			msg.swap( m_msgBuffers[enBufferType].front() );
			m_msgBuffers[enBufferType].pop_front();
		}
	}

	//clear all pending msgs
	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::popAllPendingMsg( EN_BUFFER_TYPE enBufferType )
	{
		mutex::scoped_lock lock( m_msgMutex[enBufferType] );
		CONTAINER_TYPE& listMsg = m_msgBuffers[enBufferType];
		listMsg.splice( listMsg.end(), m_msgBuffers[enBufferType] );
	}

	//can send data or not(just put into send buffer)
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::isAllowSend() const
	{
		return !m_bIsSuspendSendMsg;
	}

	//generally, you need not re-write this for link broken judgment(tcp)
	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::onSendError( const error_code& ec )
	{
		unified_out::error_out( "send msg error: %d %s", ec.value(), ec.message().data() );
	}
	
#ifdef WANT_MSG_SEND_NOTIFY
	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::onMsgSend( MSGTYPE& msg )
	{
	}
#endif

#ifdef WANT_ALL_MSG_SEND_NOTIFY
	//send buffer goes empty, msg remain in packed
	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::onAllMsgSend( MSGTYPE& msg )
	{
	}
#endif

	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::onTimer( unsigned int id, const void* pUserData )
	{
		switch ( id )
		{
		case DISPATCH_TIMERID: //delay put msgs into recv buffer because of recv buffer overflow
			dispatchMsg();
			break;
		case SUSPEND_DISPATCH_TIMERID: //suspend dispatch msgs
			doDispatchMsg( true );
			break;
		case POST_TIMERID:
			{
				mutex::scoped_lock lock( getPostMutex() );
				{
					mutex::scoped_lock lock( getSendMutex() );
					if ( spliceContainer( getSendBuffer(), getPostBuffer() ) )
					{
						doSendMsg();
					}
				}
				bool empty = getPostBuffer().empty()
				m_bIsPosting = !empty;
				lock.unlock();
				if ( empty )
				{
					doDispatchMsg( true );
				}
				return !empty; //continue the timer if not empty
			}
			break;
		case 3: case 4: case 5: case 6: case 7: case 8: case 9: //reserved
			break;
		default:
			return XAsioTimer::onTimer(id, user_data);
			break;
		}

		return false;
	}

	//can only be invoked after socket closed
	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::dispatchAllMsgDirectly()
	{
		suspenDispatchMsg( false );
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::dispatchMsg()
	{
#ifndef FORCE_TO_USE_MSG_RECV_BUFFER
		bool dispatch = false;
		for ( auto it = std::begin( getTempBuffer() ); 
			!m_bIsSuspendDispatchMsg && !m_bIsPosting && it != std::end( getTempBuffer() ); )
		{
			if ( !onMsg( *it ) )
			{
				temp_msg_buffer.erase(iitter++);
			}
			else
			{
				mutex::scoped_lock lock( getRecvMutex() );
				auto msgNum = getRecvBuffer().size();
				if ( msgNum < MAX_MSG_NUM ) //msg recv buffer available
				{
					dispatch = true;
					getRecvBuffer().splice( std::end( getRecvBuffer() ), getTempBuffer(), it++ );
				}
				else
				{
					it++;
				}
			}
			if ( dispatch )
			{
				do_dispatch_msg(true);
			}
		}
#else
		if ( !getTempBuffer().empty() )
		{
			mutex::scoped_lock lock( getRecvMutex() );
			if ( spliceContainer( getRecvBuffer(), getTempBuffer() ) )
			{
				doDispatchMsg( false );
			}
		}
#endif
		if ( getTempBuffer().empty() )
		{
			doStart(); //recv msg sequentially, that means second recv only after first recv success
		}
		else
		{
			setTimer( DISPATCH_TIMERID, 50, nullptr );
		}
	}

	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::doMsgHandler()
	{
		onMsgHandler( m_lastDispatchMsg ); //must before next msg dispatch to keep sequence
		mutex::scoped_lock lock( getRecvMutex() );
		m_bIsDispatching = false;
		//dispatch msg sequentially, that means second dispatch only after first dispatch success
		doDispatchMsg( false );
	}

	//must mutex recv_msg_buffer before invoke this function
	template<typename MSGTYPE, typename SOCKET>
	void XAsioSocket<typename MSGTYPE, typename SOCKET>::doDispatchMsg( bool needLock )
	{
		mutex::scoped_lock lock;
		if ( needLock )
		{
			lock = mutex::scoped_lock( getRecvMutex() );
		}
		if ( m_bIsSuspendDispatchMsg )
		{
			if ( !m_bIsDispatching && !getRecvBuffer().empty() )
			{
				setTimer( DISPATCH_TIMERID, 24 * 60 * 60 * 1000, nullptr ); //one day
			}
		}
		else if ( !m_bIsPosting )
		{
			auto& ioService = ST_THIS get_io_service();
			auto isDispatchAll = false;
			if ( io_service_.stopped() )
			{
				m_bIsDispatching = false;
				isDispatchAll = false;
			}
			else if ( !isDispatchAll )
			{
				if ( !ST_THIS is_open() )
				{
					dispatch_all = true;
				}
				else if ( !getRecvBuffer().empty() )
				{
					dispatching = true;
					m_lastDispatchMsg.swap( getRecvBuffer().front() );
					ioService.post( boost::bind(&XAsioSocket::doMsgHandler, this ) );
					getRecvBuffer().pop_front();
				}
			}

			if ( isDispatchAll )
			{
#ifdef FORCE_TO_USE_MSG_RECV_BUFFER
				getRecvBuffer().splice( std::end( getRecvBuffer() ), getTempBuffer() );
#endif
				//the msgs in temp_msg_buffer are discarded, it's very hard to resolve this defect,
				//so, please be very carefully if you decide to resolve this issue;
				//the biggest problem is calling force_close in on_msg.
				XASIO::forEachAll( recv_msg_buffer, boost::bind(&XAsioSocket::onMsgHandler, this, _1 ) );
				recv_msg_buffer.clear();
			}
		}
	}

	//must mutex send_msg_buffer before invoke this function
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::doSendMsgDirectly( MSGTYPE& msg )
	{
		if ( !msg.empty() )
		{
			getSendBuffer().resize( getSendBuffer().size() + 1 );
			getSendBuffer().back().swap( msg );
			doSendMsg();
		}
		return true;
	}

	//must mutex post_msg_buffer before invoke this function
	template<typename MSGTYPE, typename SOCKET>
	bool XAsioSocket<typename MSGTYPE, typename SOCKET>::doPostMsgDirectly( MSGTYPE& msg )
	{
		if ( !msg.empty() )
		{
			getPostBuffer().resize( getPostBuffer().size() + 1);
			getPostBuffer().back().swap( msg );
			if ( !m_bIsPosting )
			{
				m_bIsPosting = true;
				setTimer( POST_TIMERID, 50, nullptr );
			}
		}
		return true;
	}
}
*/