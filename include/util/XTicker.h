/**
 * ¼ÆÊ±Æ÷
 */
#pragma once

#ifdef WIN32

#include <windows.h>  

namespace XGAME
{
	class XTicker
	{
	public:
		static LONGLONG	getTickCounter();

	public:
		XTicker();

		void start();

		void end();

		void reset();

	public:  
		LONGLONG			m_costTime;

	private:  
		LONGLONG			m_freq;
		LARGE_INTEGER		m_begin;  
		LARGE_INTEGER		m_end;  
	};
}

#else

#include <sys/time.h>  
#include <unistd.h>  

namespace XGAME
{
	class XTicker
	{
	public:
		static long long getTickCounter();

	public:  
		XTicker();

		void start();

		void end();

		void reset();

	public:  
		long long	m_costTime;

	private:  
		timeval		m_begin;  
		timeval		m_end;
	};
}

#endif