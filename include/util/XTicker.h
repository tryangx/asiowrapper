/**
 * 计时器
 */
#pragma once

#include "XApi.h"

#ifdef WIN32

#include <windows.h>  

namespace XGAME
{
	class XGAME_API XTicker
	{
	public:
		/**
		 * 得到运行时间(微秒)
		 */
		static LONGLONG		getTickCounter();
		static LONGLONG		getLastTickCounter();

	private:
		static long long	m_lastTickCounter;

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
		/**
		 * 得到运行时间(微秒)
		 */
		static long long	getTickCounter();
		static long long	getLastTickCounter();

	private:
		static long long	m_lastTickCounter;

	public:  
		XTicker();

		void start();

		void end();

		void reset();

	protected:  
		long long	m_costTime;

	private:  
		timeval		m_begin;  
		timeval		m_end;
	};
}

#endif