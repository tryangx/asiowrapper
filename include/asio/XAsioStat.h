/**
 * ������ͨ״̬
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
		 * ��ʼͳ��
		 */
		void			start();

		/**
		 * ����ͳ��
		 */
		void			reset();

		/**
		 * ���õ�λͳ��ʱ��
		 */
		void			setInterval( size_t time );

		/**
		 * ������
		 */
		void			connect();

		/**
		 * �Ͽ�����
		 */
		void			disconnect();

		/**
		 * ����
		 */
		void			recv( size_t size );

		/**
		 * ����
		 */
		void			send( size_t size );

		/**
		 * ��λʱ��ˢ��
		 */
		void			update();

		/**
		 * ���������ܴ�С
		 */
		size_t			getTotalRecvSize();
		/**
		 * ���������ܴ�С
		 */
		size_t			getTotalSendSize();
		/**
		 * ��λʱ���ڽ������ݴ�С
		 */
		size_t			getPeriodRecvSize();
		/**
		 * ��λʱ���ڷ������ݴ�С
		 */
		size_t			getPeriodSendSize();
		/**
		 * �������ݴ���
		 */
		size_t			getTotalRecvTime();
		/**
		 * �������ݴ���
		 */
		size_t			getTotalSendTime();
		/**
		 * ��λʱ���ڽ������ݴ���
		 */
		size_t			getPeriodRecvTime();
		/**
		 * ��λʱ���ڷ������ݴ���
		 */
		size_t			getPeriodSendTime();

	private:
		//��ʼʱ��
		long long		m_startTime;

		//��λʱ��
		size_t			m_periodTime;

		//��������
		size_t			m_iTotalConnect;
		//�ܶϿ���
		size_t			m_iTotalDisconnect;
		//��ǰ������
		size_t			m_iCurConnect;
		//���������
		size_t			m_iMaxConnect;

		//���յ����ݴ�С
		size_t			m_iTotalRecvSize;		
		//���յ�����
		size_t			m_iTotalRecvTimes;
		//��λʱ���ڽ������ݴ�С
		size_t			m_iPeriodRecvSize;
		//��λʱ���ڽ��մ���
		size_t			m_iPeriodRecvTimes;

		//�������ݴ�С
		size_t			m_iTotalSendSize;
		//�������ݴ���
		size_t			m_iTotalSendTimes;
		//��λʱ���ڷ������ݴ�С
		size_t			m_iPeriodSendSize;
		//��λʱ���ڷ������ݴ���
		size_t			m_iPeriodSendTimes;
	};
}