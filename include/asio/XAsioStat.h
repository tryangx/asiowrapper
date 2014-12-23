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

		/**
		 * 接收数据总大小
		 */
		size_t			getTotalRecvSize();
		/**
		 * 发送数据总大小
		 */
		size_t			getTotalSendSize();
		/**
		 * 单位时间内接收数据大小
		 */
		size_t			getPeriodRecvSize();
		/**
		 * 单位时间内发送数据大小
		 */
		size_t			getPeriodSendSize();
		/**
		 * 接收数据次数
		 */
		size_t			getTotalRecvTime();
		/**
		 * 发送数据次数
		 */
		size_t			getTotalSendTime();
		/**
		 * 单位时间内接收数据次数
		 */
		size_t			getPeriodRecvTime();
		/**
		 * 单位时间内发送数据次数
		 */
		size_t			getPeriodSendTime();

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
	};
}