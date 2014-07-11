/*
#include "XAsioTCPSocket.h"

namespace XASIO
{
#ifndef GRACEFUL_CLOSE_MAX_DURATION
#define GRACEFUL_CLOSE_MAX_DURATION	5 //seconds, max waiting seconds while graceful closing
#endif

#ifndef DEFAULT_UNPACKER
#define DEFAULT_UNPACKER unpacker
#endif

	XAsioTCPSocket::XAsioTCPSocket(  io_service& ioService ) : XAsioSocket( ioService ), 
		m_unpacker( boost::make_shared<DEFAULT_UNPACKER>() )
	{
		resetState();
	}

	//reset all, be ensure that there's no any operations performed on this st_tcp_socket when invoke it
	void XAsioTCPSocket::reset()
	{
		resetState();
		clearBuffer();
	}
	void XAsioTCPSocket::resetState()
	{
		resetUnpackerState();
		XAsioSocket::resetState();
		m_bIsClosing = false;
	}

	void XAsioTCPSocket::disconnect()
	{
		forceClose();
	}
	void XAsioTCPSocket::forceClose()
	{
		cleanUp();
	}
	void XAsioTCPSocket::gracefulClose() //will block until closing success or timeout
	{
		m_bIsClosing = true;

		error_code ec;
		shutdown( tcp::socket::shutdown_send, ec );
		if (ec) //graceful disconnecting is impossible
		{
			cleanUp();
		}
		else
		{
			auto numLoop = GRACEFUL_CLOSE_MAX_DURATION * 100; //seconds to 10 milliseconds
			while (--numLoop >= 0 && closing)
				this_thread::sleep(get_system_time() + posix_time::milliseconds(10));
			if (numLoop < 0) //graceful disconnecting is impossible
			{
				cleanUp();
			}
		}
	}

	bool XAsioTCPSocket::is_closing() const
	{
		return m_bIsClosing;
	}

	//get or change the unpacker at runtime
	boost::shared_ptr<XAsioBasePacker> XAsioTCPSocket::getPacker() const
	{
		return m_unpacker;
	}
	void XAsioTCPSocket::setPacker( const boost::shared_ptr<XAsioBasePacker>& unpacker )
	{
		m_unpacker = unpacker;
	}

	bool XAsioTCPSocket::sendMsg( const char* const pStr[], const size_t len[], size_t num, bool isOverflow )
	{

	}
	bool XAsioTCPSocket::sendMsgNative( const char* const pStr[], const size_t len[], size_t num, bool isOverflow )
	{

	}
	//guarantee send msg successfully even if can_overflow equal to false
	//success at here just means put the msg into st_tcp_socket's send buffer
	bool XAsioTCPSocket::sendMsgSafe( const char* const pStr[], const size_t len[], size_t num, bool isOverflow )
	{

	}
	bool XAsioTCPSocket::sendMsgNativeSafe( const char* const pStr[], const size_t len[], size_t num, bool isOverflow )
	{

	}
	//like safe_send_msg and safe_send_native_msg, but non-block
	bool XAsioTCPSocket::postMsg( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false )
	{

	}
	bool XAsioTCPSocket::postMsgNative( const char* const pStr[], const size_t len[], size_t num, bool isOverflow )
	{

	}

	void XAsioTCPSocket::showInfo( const char* head, const char* tail )
	{
		error_code ec;
		auto ep = remote_endpoint(ec);
		if (!ec)
		{
			unified_out::info_out( "%s %s:%hu %s", head, ep.address().to_string().c_str(), ep.port(), tail );
		}
	}

	//must mutex send_msg_buffer before invoke this function
	bool XAsioTCPSocket::doSendMsg()
	{
		if ( !isAllowSend() || get_io_service().stopped() )
		{
			m_bIsSending = false;
		}
		else if ( !m_bIsSending && !send_msg_buffer.empty() )
		{
			m_bIsSending = true;
			m_lastSendMsg.swap(send_msg_buffer.front());
			async_write(*this, buffer(last_send_msg), boost::bind(&st_tcp_socket::send_handler, this,
				placeholders::error, placeholders::bytes_transferred));
			send_msg_buffer.pop_front();
		}

		return sending;
	}

	bool XAsioTCPSocket::is_send_allowed() const
	{
		return !isClosing() && XAsioSocket<MSG_TYPE, tcp::socket>::isAllowSend();
	}
	
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
	bool XAsioTCPSocket::on_msg(MSG_TYPE& msg)
	{
		unified_out::debug_out("recv(" size_t_format "): %s", msg.size(), msg.data()); return false;
	}
#endif

	//handling msg at here will not block msg receiving
	//if on_msg() return false, this function will not be invoked due to no msgs need to dispatch
	//notice: the msg is unpacked, using inconstant is for the convenience of swapping
	void XAsioTCPSocket::on_msg_handle(msg_type& msg)
	{unified_out::debug_out("recv(" size_t_format "): %s", msg.size(), msg.data());}

	//start the async read
	//it's child's responsibility to invoke this properly,
	//because st_tcp_socket doesn't know any of the connection status
	void XAsioTCPSocket::do_recv_msg()
	{
		auto recv_buff = unpacker_->prepare_next_recv();
		if (buffer_size(recv_buff) > 0)
			async_read(*this, recv_buff, boost::bind(&i_unpacker::completion_condition, unpacker_,
			placeholders::error, placeholders::bytes_transferred),
			boost::bind(&st_tcp_socket::recv_handler, this,
			placeholders::error, placeholders::bytes_transferred));
	}

	//reset unpacker's state, generally used when unpack error occur
	void XAsioTCPSocket::reset_unpacker_state() {unpacker_->reset_unpacker_state();}

	void XAsioTCPSocket::clean_up()
	{
		if (is_open())
		{
			error_code ec;
			shutdown(tcp::socket::shutdown_both, ec);
			close(ec);
		}

		stop_all_timer();
		direct_dispatch_all_msg();
		reset_state();
	}

	void XAsioTCPSocket::recv_handler(const error_code& ec, size_t bytes_transferred)
	{
		if (!ec && bytes_transferred > 0)
		{
			auto unpack_ok = unpacker_->parse_msg(bytes_transferred, temp_msg_buffer);
			dispatch_msg();

			if (!unpack_ok)
				on_unpack_error();
		}
		else
			on_recv_error(ec);
	}

	void XAsioTCPSocket::send_handler(const error_code& ec, size_t bytes_transferred)
	{
		if (!ec)
		{
			assert(bytes_transferred > 0);
#ifdef WANT_MSG_SEND_NOTIFY
			on_msg_send(last_send_msg);
#endif
		}
		else
			on_send_error(ec);

		mutex::scoped_lock lock(send_msg_buffer_mutex);
		sending = false;

		//send msg sequentially, that means second send only after first send success
		if (!ec && !do_send_msg())
		{
#ifdef WANT_ALL_MSG_SEND_NOTIFY
			lock.unlock();
			on_all_msg_send(last_send_msg);
#endif
		}
	}
};
*/