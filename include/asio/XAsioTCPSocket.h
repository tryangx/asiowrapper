/*
#pragma once

#include "XAsioSocket.h"
#include "XAsioPacker.h"

namespace XASIO
{
	typedef std::string			MSG_TYPE;
	typedef const MSG_TYPE		MSG_CTYPE;

	class XAsioTCPSocket : public XAsioSocket< MSG_TYPE, tcp::socket >
	{
	protected:
		XAsioTCPSocket( io_service& ioService );

	public:
		void	resetAll();
		void	resetState();

		void	disconnect();

		void	forceClose();
		void	gracefulClose();
		bool	isClosing() const;

		boost::shared_ptr<XAsioBasePacker>	getPacker() const;
		void	setPacker( const boost::shared_ptr<XAsioBasePacker>& myPacker );

	public:
		//-----------------------------------
		//	msg sending interface
		bool	sendMsg( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false );
		bool	sendMsgNative( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false );
		//guarantee send msg successfully even if can_overflow equal to false
		//success at here just means put the msg into st_tcp_socket's send buffer
		bool	sendMsgSafe( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false );
		bool	sendMsgNativeSafe( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false );
		//like safe_send_msg and safe_send_native_msg, but non-block
		bool	postMsg( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false );
		bool	postMsgNative( const char* const pStr[], const size_t len[], size_t num, bool isOverflow = false );

		void	showInfo( const char* head, const char* tail );

	protected:
		virtual bool	doSendMsg();

		virtual	bool	isAllowSend() const;

		virtual void	onUnpackError() = 0;
		virtual void	onRecvError() = 0;

#ifndef FORCE_TO_USE_MSG_RECV_BUFFER		
		virtual bool	onMsg( MSG_TYPE& msg );
#endif

		virtual void	onMsgHandler( XASIO.MSG_TYPE& msg );

		void	doRecvMsg();

		void	resetUnpackerState();

		void	clearUp();

		void	onRecvHandler( const error_code& ec, size_t bytes_transferred );
		void	onSendHandler( const error_code& ec, size_t bytes_transferred );

	protected:
		boost::shared_ptr<XAsioBasePacker>		m_unpacker;
		bool	m_bIsClosing;
	};
}
*/