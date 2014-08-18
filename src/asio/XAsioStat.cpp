#include "../../include/asio/XAsioStat.h"
#include "../../include/util/XTicker.h"
#include <string>

namespace XGAME
{
	XAsioStat::XAsioStat()
	{
		memset( m_szText, 0, sizeof(m_szText) );
		reset();
	}

	void XAsioStat::start()
	{
		m_startTime = XTicker::getTickCounter();
		reset();
	}

	void XAsioStat::reset()
	{
		m_iTotalConnect			= 0;
		m_iTotalDisconnect		= 0;
		m_iCurConnect			= 0;
		m_iMaxConnect			= 0;
		m_iTotalRecvSize		= 0;
		m_iTotalRecvTimes		= 0;
		m_iPeriodRecvSize		= 0;
		m_iPeriodRecvTimes		= 0;
		m_iTotalSendSize		= 0;
		m_iTotalSendTimes		= 0;
		m_iPeriodSendSize		= 0;
		m_iPeriodSendTimes		= 0;
	}

	void XAsioStat::setInterval( size_t time )
	{
		m_periodTime = time;
	}

	void XAsioStat::connect()
	{
		m_iTotalConnect++;
		m_iCurConnect++;
		m_iMaxConnect = m_iMaxConnect > m_iCurConnect ? m_iMaxConnect : m_iCurConnect;
	}

	void XAsioStat::disconnect()
	{
		m_iTotalDisconnect++;
		m_iCurConnect--;
	}

	void XAsioStat::recv( size_t size )
	{
		m_iTotalRecvSize += size;
		m_iTotalRecvTimes++;
		m_iPeriodRecvSize += size;
		m_iPeriodRecvTimes++;
	}

	void XAsioStat::send( size_t size )
	{
		m_iTotalSendSize += size;
		m_iTotalSendTimes++;
		m_iPeriodSendSize += size;
		m_iPeriodSendTimes++;
	}

	void XAsioStat::update()
	{
		m_iPeriodRecvSize	= 0;
		m_iPeriodRecvTimes	= 0;
		m_iPeriodSendSize	= 0;
		m_iPeriodSendTimes	= 0;
	}

	size_t XAsioStat::getTotalRecvSize() { return m_iTotalRecvSize; }
	size_t XAsioStat::getTotalSendSize() { return m_iTotalSendSize; }
	size_t XAsioStat::getPeriodRecvSize() { return m_iPeriodRecvSize; }
	size_t XAsioStat::getPeriodSendSize() { return m_iPeriodRecvTimes; }
	size_t XAsioStat::getTotalRecvTime() { return m_iTotalRecvTimes; }
	size_t XAsioStat::getTotalSendTime() { return m_iTotalSendTimes; }
	size_t XAsioStat::getPeriodRecvTime() { return m_iPeriodRecvTimes; }
	size_t XAsioStat::getPeriodSendTime() { return m_iPeriodSendTimes; }

	const char* XAsioStat::outputLog()
	{
		return m_szText;
	}
}