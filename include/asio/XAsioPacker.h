/*
#pragma once

#include <boost/array.hpp>
#include <boost/container/list.hpp>

#include "XAsioBase.h"

namespace XASIO
{
	class XAsioBasePacker
	{
	public:
		static size_t	getMaxMsgSize();

		virtual std::string pack( const char* const pStr[], const size_t len[], size_t num, bool native = false ) = 0;
	};

	class XAsioPacker : public XAsioBasePacker
	{
	public:
		virtual std::string pack( const char* const pStr[], const size_t len[], size_t num, bool native = false );
	};

	class XBaseAsioUnpacker
	{
	public:
		//reset unpacker
		virtual void	reset() = 0;

		//return packer total size
		virtual size_t	getBuffSize() const { return 0; }

		//return current position
		virtual size_t	getCurPos() const { return -1; }
				
		//parse message
		virtual bool	parse( size_t transBytes, container::list<std::string>& msgContainer ) = 0;

		//return whether message enough to parse
		virtual size_t	isCompleted( const error_code& ec, size_t transBytes ) = 0;

		//prepare for next receive
		virtual mutable_buffers_1 prepareRecv() = 0;
	};

	class XAsioUnpacker : public XBaseAsioUnpacker
	{
	public:
		XAsioUnpacker();

	public:
		virtual size_t	getBuffSize() const;

		virtual size_t	getCurPos() const;

		virtual void	reset();

		virtual bool	parse( size_t transBytes, container::list<std::string>& msgContainer );

		virtual size_t	isCompleted( const error_code& ec, size_t transBytes );

		virtual mutable_buffers_1 prepareRecv();

	private:
		size_t		m_iCurPos;		//-1 means head has not received, so, doesn't know the whole msg length.
		size_t		m_iDataSize;	//include head
		boost::array<char, MAX_MSG_LEN>		m_buffers;	
	};
}
*/