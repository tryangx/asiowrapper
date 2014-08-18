/**
 * 网络连通状态
 *
 */
#pragma once

namespace XGAME
{
#define STAT_LOG_LENGTH		4096

	class XAsioStat
	{
	public:
		XAsioStat();
		
		/**
		 * 开始统计
		 */
		void			start();

		/**
		 * 重置统计
		 */
		void			reset();

		/**
		 * 设置单位统计时间
		 */
		void			setInterval( size_t time );

		/**
		 * 连接上
		 */
		void			connect();

		/**
		 * 断开连接
		 */
		void			disconnect();

		/**
		 * 接收
		 */
		void			recv( size_t size );

		/**
		 * 发送
		 */
		void			send( size_t size );

		/**
		 * 单位时间刷新
		 */
		void			update();

		size_t			getTotalRecvSize();
		size_t			getTotalSendSize();
		size_t			getPeriodRecvSize();
		size_t			getPeriodSendSize();
		size_t			getTotalRecvTime();
		size_t			getTotalSendTime();
		size_t			getPeriodRecvTime();
		size_t			getPeriodSendTime();

		const char*		outputLog();

	private:
		//开始时间
		long long		m_startTime;

		//单位时间
		size_t			m_periodTime;

		//总连接数
		size_t			m_iTotalConnect;
		//总断开数
		size_t			m_iTotalDisconnect;
		//当前连接数
		size_t			m_iCurConnect;
		//最大连接数
		size_t			m_iMaxConnect;

		//接收到内容大小
		size_t			m_iTotalRecvSize;		
		//接收到次数
		size_t			m_iTotalRecvTimes;
		//单位时间内接收内容大小
		size_t			m_iPeriodRecvSize;
		//单位时间内接收次数
		size_t			m_iPeriodRecvTimes;

		//发送内容大小
		size_t			m_iTotalSendSize;
		//发送内容次数
		size_t			m_iTotalSendTimes;
		//单位时间内发送内容大小
		size_t			m_iPeriodSendSize;
		//单位时间内发送内容次数
		size_t			m_iPeriodSendTimes;

		char			m_szText[STAT_LOG_LENGTH];
	};
}