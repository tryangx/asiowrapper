#include "util/XTicker.h"

#ifdef WIN32

namespace XGAME
{
	LONGLONG XTicker::getTickCounter()
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency( &freq );

		LARGE_INTEGER tmp;
		QueryPerformanceCounter( &tmp );
		LONGLONG tick = (long long)( tmp.QuadPart * 1000000.0 / freq.QuadPart );  
		return tick;
	}

	XTicker::XTicker()  
	{  
		LARGE_INTEGER tmp;
		QueryPerformanceFrequency( &tmp );
		m_freq = tmp.QuadPart;
		m_costTime = 0;  
	}  

	void XTicker::start()
	{
		QueryPerformanceCounter( &m_begin );
	}

	void XTicker::end()
	{  
		QueryPerformanceCounter( &m_end );
		m_costTime = (long long)( ( m_end.QuadPart - m_begin.QuadPart ) * 1000000.0 / m_freq );  
	}  

	void XTicker::reset()
	{  
		m_costTime = 0;  
	}
}

#else

namespace XGAME
{
	long long XTicker::getTickCounter()
	{
		timeval t;
		gettimeofday( &t, NULL );
		long long tick = t.tv_sec * 1000000ll + t.tv_usec;
	}

	XTicker::XTicker()  
	{  
		m_costTime = 0;  
	}  
	void XTicker::start()
	{  
		gettimeofday( &m_begin, NULL );
	}  

	void XTicker::end()
	{  
		gettimeofday( &m_end, NULL );
		m_costTime = ( m_end.tv_sec - m_begin.tv_sec ) * 1000000ll + m_end.tv_usec - m_begin.tv_usec;  
	}

	void XTicker::reset()
	{  
		m_costTime = 0;  
	}
}

#endif