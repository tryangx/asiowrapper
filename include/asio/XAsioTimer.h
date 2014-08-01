#pragma once

#include <boost/container/set.hpp>

#include "XAsioBase.h"

namespace XASIO
{
	class XAsioTimer
	{
	protected:

		//��ʱ����Ϣ
		struct stTimerInfo
		{
			enum enTimerStatus
			{
				TIMER_STOP,			//�Ƿ�ֹͣ
				TIMER_RUN,			//�Ƿ�����
			};

			//��ʱ�����
			unsigned int	_id;
			
			//��ʱ��״̬
			enTimerStatus	_enStatus;
			
			//��ʱ�����ʱ��
			size_t			_milliseconds;
			
			//�������
			const void*		_pUserData;

			//BOOST timer
			boost::shared_ptr<deadline_timer>	_timer;

			/**
			 * ID�ж�
			 */
			bool operator <( const stTimerInfo& other ) const { return _id < other._id; }
		};

	public:
		typedef	stTimerInfo					TIMER_TYPE;
		typedef const stTimerInfo			TIMER_CTYPE;
		typedef container::set<TIMER_TYPE>	CONTAINER_TIMER;

	protected:
		XAsioTimer( io_service& ioService );

		virtual ~XAsioTimer();

	public:
		//���ö�ʱ��
		void	setTimer( unsigned int id, size_t milliseconds, const void* pUserData );

		//ֹͣ��ʱ��
		void	stopTimer( unsigned int id );

		//ֹͣȫ����ʱ��
		void	stopAllTimer();

		FOREACH_ALL_MUTEX( m_timerContainer, m_timerMutex );
		FOREVERY_ONE_MUTEX( m_timerContainer, m_timerMutex );

	protected:

		/**
		 * ��ʱ��������������Ӧ
		 * @return ������ʱ��ʱ��������
		 */
		virtual	bool onTimer( unsigned int id, const void* pUserData );

	protected:
		void	startTimer( TIMER_CTYPE& st );

		void	stopTimer( TIMER_TYPE& st );

		void	onTimerHandler( const error_code& ec, TIMER_CTYPE& st );

	protected:
		io_service&		m_refIoService;

		CONTAINER_TIMER	m_timerContainer;

		boost::mutex	m_timerMutex;
	};
}