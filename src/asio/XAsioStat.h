/**
 * ������ͨ״̬
 *
 */
#pragma once

#include "../util/XSingleton.h"

namespace XGAME
{
	class XAsioStat
	{
	public:
		XAsioStat() {}
		
		void			recv( size_t size ) { m_iRecvSize += size; }

	private:
		//��ʼʱ��
		size_t			m_startTime;

		size_t			m_iRecvSize;
		size_t			m_iRecvTimes;
		size_t			m_iSendSize;
		size_t			m_iSendTimes;
	};

	typedef XSingleton<XAsioStat>	XAsioStatAgent;
}