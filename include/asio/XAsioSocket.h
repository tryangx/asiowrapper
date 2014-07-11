#pragma once

#include <boost/container/list.hpp>

#include "XAsioHelper.h"
#include "XAsioPacker.h"

#ifndef DEFAULT_PACKER
#define DEFAULT_PACKER	XAsioPacker
#endif

using namespace boost::asio::ip;

namespace XASIO
{
	enum EN_BUFFER_TYPE
	{
		POST_BUFFER,
		SEND_BUFFER,
		RECV_BUFFER,
		TEMP_BUFFER,

		BUFFER_COUNT	= 4,
		MUTEX_COUNT		= 3,
	};

	template<typename MSGTYPE, typename SOCKET>
	class XAsioSocket : public SOCKET, public XAsioTimer
	{
	public:
		typedef container::list<MSGTYPE>	CONTAINER_TYPE;

	protected:
		XAsioSocket( io_service& ioService );

	public:
		//message packer
		boost::shared_ptr<XAsioBasePacker>	getPacker() const;
		void	setPacker( const boost::shared_ptr<XAsioBasePacker>& myPacker );

		//is socket running
		bool	isStarted() const;

		//is send-buffer available
		bool	isSendBufferAvailable();
		
		//
		bool	isSuspendSendMsg() const;

		bool	isSuspendDispatchMsg() const;

		//start socket
		void	start();
		
		bool	sendMsg();
		void	suspendSendMsg( bool suspend );
		void	suspenDispatchMsg( bool suspend );

		bool	sendMsgDirectly( const MSGTYPE& msg, bool canOverflow = false );
		bool	sendMsgDirectly( MSGTYPE&& msg, bool canOverflow = false );

		bool	postMsgDirectly( const MSGTYPE& msg, bool canOverflow = false );
		bool	postMsgDirectly( MSGTYPE&& msg, bool canOverflow = false );

		//--------------------------------------
		//  pending message relative
		size_t	getPendingMsgNum( EN_BUFFER_TYPE enBufferType = SEND_BUFFER );
		void	peekFirstPendingMsg( MSGTYPE& msg, EN_BUFFER_TYPE enBufferType = SEND_BUFFER );
		void	popFirstPendingMsg( MSGTYPE& msg, EN_BUFFER_TYPE enBufferType = SEND_BUFFER );
		void	popAllPendingMsg( EN_BUFFER_TYPE enBufferType = SEND_BUFFER );

	protected:
		void	resetState();

		void	clearBuffers();

		__inline CONTAINER_TYPE&	getPostBuffer() const { return m_msgBuffers[POST_BUFFER]; }
		__inline CONTAINER_TYPE&	getSendBuffer() const { return m_msgBuffers[SEND_BUFFER]; }
		__inline CONTAINER_TYPE&	getRecvBuffer() const { return m_msgBuffers[RECV_BUFFER]; }
		__inline CONTAINER_TYPE&	getTempBuffer() const { return m_msgBuffers[TEMP_BUFFER]; }

		__inline mutex&	getPostMutex() const { return m_msgMutexs[POST_BUFFER]; }
		__inline mutex&	getSendMutex() const { return m_msgMutexs[SEND_BUFFER]; }
		__inline mutex&	getRecvMutex() const { return m_msgMutexs[RECV_BUFFER]; }

	protected:
		virtual bool	onTimer( unsigned int id, const void* pUserData );
		
		virtual bool	isAllowSend() const;

		virtual bool	doStart() = 0;
		
		virtual bool	doSendMsg() = 0;
				
#ifndef FORCE_TO_USE_MSG_RECV_BUFFER
		//if you want to use your own recv buffer, you can move the msg to your own recv buffer,
		//and return false, then, handle the msg as your own strategy(may be you'll need a msg dispatch thread)
		//or, you can handle the msg at here and return false, but this will reduce efficiency(
		//because this msg handling block the next msg receiving on the same st_tcp_socket) unless you can
		//handle the msg very fast(which will inversely more efficient, because msg recv buffer and msg dispatching
		//are not needed any more).
		//
		//return true means use the msg recv buffer, you must handle the msgs in on_msg_handle()
		//notice: on_msg_handle() will not be invoked from within this function
		//
		//notice: the msg is unpacked, using inconstant is for the convenience of swapping
		virtual bool	onMsg( MSGTYPE& msg ) = 0;
#endif

		//handling msg at here will not block msg receiving
		//if on_msg() return false, this function will not be invoked due to no msgs need to dispatch
		//notice: the msg is unpacked, using inconstant is for the convenience of swapping
		virtual void	onMsgHandler( MSGTYPE& msg ) = 0;

#ifdef WANT_MSG_SEND_NOTIFY
		//one msg has sent to the kernel buffer, msg is the right msg(remain in packed)
		//if the msg is custom packed, then obviously you know it
		//or the msg is packed as: len(2 bytes) + original msg, see st_asio_wrapper::packer for more details
		virtual void	onMsgSend( MSGTYPE& msg );
#endif
#ifdef WANT_ALL_MSG_SEND_NOTIFY
		virtual void	onAllMsgSend( MSGTYPE& msg );
#endif
		virtual void	onSendError( const error_code& ec );		

		void	dispatchAllMsgDirectly();
		void	dispatchMsg();

		void	doMsgHandler();
		void	doDispatchMsg( bool needLock );
		bool	doSendMsgDirectly( MSGTYPE& msg );
		bool	doPostMsgDirectly( MSGTYPE& msg );

	protected:
		MSGTYPE			m_lastSendMsg;
		MSGTYPE			m_lastDispatchMsg;
		boost::shared_ptr<XAsioBasePacker>	m_packer;

		CONTAINER_TYPE	m_msgBuffers[BUFFER_COUNT];
		mutex			m_msgMutexs[MUTEX_COUNT];

		bool			m_bIsPosting;
		bool			m_bIsSending;
		bool			m_bIsSuspendSendMsg;
		bool			m_bIsDispatching;
		bool			m_bIsSuspendDispatchMsg;

		bool			m_bIsStarted;
		mutex			m_mutexStarted;
	};

} //namespace