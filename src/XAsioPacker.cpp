/*
#include "XAsioPacker.h"

#ifdef HUGE_MSG
#define HEAD_TYPE	uint32_t
#define HEAD_H2N	htonl
#define HEAD_N2H	ntohl
#else
#define HEAD_TYPE	uint16_t
#define HEAD_H2N	htons
#define HEAD_N2H	ntohs
#endif

#define HEAD_LEN	( sizeof(HEAD_TYPE) )

using namespace XASIO;

namespace XASIO
{
	//---------------------------------------------
	// packer

	size_t XAsioBasePacker::getMaxMsgSize()
	{
		return MAX_MSG_LEN - HEAD_LEN;
	}

	std::string XAsioPacker::pack( const char* const pStr[], const size_t len[], size_t num, bool native )
	{
		std::string str;
		if ( nullptr == pStr || nullptr == len )
		{
			return str;
		}
		size_t totalLen = native ? 0 : HEAD_LEN;
		auto lastTotalLen = totalLen;

		for ( size_t i = 0; i < num; i++ )
		{
			if ( nullptr != pStr[i] )
			{
				totalLen += len[i];
				if ( lastTotalLen > totalLen || totalLen > MAX_MSG_LEN ) //overflow
				{
					unified_out::error_out( "pack message failed: length exceeds the MAX_MSG_LEN!" );
					return str;
				}
				lastTotalLen = totalLen;
			}
		}
		if ( totalLen > ( native ? 0 : HEAD_LEN ) )
		{
			if ( !native )
			{
				auto headLen = (HEAD_TYPE)totalLen;
				if ( totalLen!= headLen )
				{
					unified_out::error_out( "pack msg failed: length exceeds the header's range!" );
					return str;
				}

				headLen = HEAD_H2N(headLen);
				str.reserve( totalLen );
				str.append( (const char*) &headLen, HEAD_LEN );
			}
			else
			{
				str.reserve( totalLen );
			}

			for (size_t i = 0; i < num; ++i)
			{
				if ( nullptr != pStr[i] )
				{
					str.append( pStr[i], len[i] );
				}
			}
		}
		return str;
	}

	//---------------------------------------------
	// unpacker

	XAsioUnpacker::XAsioUnpacker()
	{
		reset();
	}

	size_t XAsioUnpacker::getBuffSize() const
	{
		return m_iDataSize;
	}

	size_t XAsioUnpacker::getCurPos() const
	{
		return m_iCurPos;
	}

	void XAsioUnpacker::reset()
	{
		m_iCurPos = -1;
		m_iDataSize = 0;
	}

	bool XAsioUnpacker::parse( size_t transBytes, container::list<std::string>& msgContainer )
	{
		m_iDataSize += transBytes;

		auto itNext = std::begin( m_buffers );
		auto isOk = true;
		while( isOk )
		{
			if ( (size_t)-1 != m_iCurPos )
			{
				if ( m_iCurPos > MAX_MSG_LEN || m_iCurPos <= HEAD_LEN )
				{
					isOk = false;
				}
				else if ( m_iDataSize >= m_iCurPos )
				{
					msgContainer.resize( msgContainer.size() + 1 );
					msgContainer.back().assign( std::next( itNext, HEAD_LEN ), m_iCurPos - HEAD_LEN );
					m_iDataSize -= m_iCurPos;
					std::advance( itNext, m_iCurPos );
					m_iCurPos = -1;
				}
				else
				{
					break;
				}
			}
			else if ( m_iDataSize >= HEAD_LEN ) //the msg's head been received, stick package found
			{
				m_iCurPos = HEAD_N2H( *(HEAD_TYPE*)itNext );
			}
			else
			{
				break;
			}
		}
		if ( !isOk )
		{
			reset();
		}
		else if ( m_iDataSize > 0 && itNext > std::begin(m_buffers) ) //left behind unparsed msg
		{
			memcpy( std::begin(m_buffers), itNext, m_iDataSize );
		}
		return isOk;
	}

	size_t XAsioUnpacker::isCompleted( const error_code& ec, size_t transBytes )
	{
		if (ec)
		{
			return 0;
		}
		auto iDataLen = m_iDataSize + transBytes;
		assert( iDataLen <= MAX_MSG_LEN );

		if ( (size_t)-1 == m_iCurPos )
		{
			if ( iDataLen >= HEAD_LEN ) //the msg's head been received
			{
				m_iCurPos = HEAD_N2H( *(HEAD_TYPE*)std::begin(m_buffers) );
				if ( m_iCurPos > MAX_MSG_LEN || m_iCurPos <= HEAD_LEN ) //invalid msg, stop reading
				{
					return 0;
				}
			}
			else
			{
				return MAX_MSG_LEN - iDataLen; //read as many as possible
			}
		}

		//read as many as possible except that we have already got a entire msg
		return iDataLen >= m_iCurPos ? 0 : MAX_MSG_LEN - iDataLen;
	}

	mutable_buffers_1 XAsioUnpacker::prepareRecv()
	{
		assert( m_iDataSize < MAX_MSG_LEN );
		return buffer( buffer(m_buffers) + m_iDataSize );
	}
}
*/