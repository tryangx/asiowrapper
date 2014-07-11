#pragma once

#include "XAsioBase.h"

#include <boost/container/list.hpp>

namespace XASIO
{
	//---------------------------
	// �����

	/**
	* 
	* ģ�����OBJECT��Ҫ�������½ӿ�
	*		isStarted() //�Ƿ�����
	*/
	template<typename OBJECT>
	class XAsioPool : public XAsioTimer
	{
	public:
		typedef boost::shared_ptr<OBJECT>				OBJECT_TYPE;
		typedef const OBJECT_TYPE						OBJECT_CTYPE;
		typedef container::list<OBJECT_TYPE>			CONTAINER_TYPE;

	public:
		/**
		 * �������
		 */
		void			allocateObject( int size );

		/**
		* ��ȡ����
		*/
		size_t			getSize();

		/**
		* ��ȡ�رյ�����
		*/
		size_t			getClosedSize();

		/**
		* ���ָ��λ�õĶ���
		*/
		OBJECT_TYPE		getAt( size_t index );
		
		/**
		* ���ȫ���ѹرյĶ���
		* @param objects ���������������������Ƴ��Ķ��󽫷��õ���������
		*/
		void			recycleObject( CONTAINER_TYPE& objects );

		/**
		* �ͷ�ָ�������Ķ���
		* @param num ����Ϊ-1���ͷ�ȫ��
		*/
		void			freeObjects( size_t num = -1 );

		FOREACH_ALL_MUTEX( m_objContainer, m_objMutex );

		FOREVERY_ONE_MUTEX( m_objContainer, m_objMutex );

	protected:
		XAsioPool( io_service& service ) : XAsioTimer( service ) {}

		/**
		* ���������
		* ���ڳ�ʼ���������ã��Զ�����رն���ȹ���
		*/
		void			start();
		void			stop();

		/**
		 * ��ȡ����
		 */
		OBJECT_TYPE		queryObject();

		/**
		* ���ö���
		*/
		OBJECT_TYPE		reuseObject();

		/**
		* ��Ӷ���
		*/
		bool			addObject( OBJECT_CTYPE& obj );

		/**
		* ���ض���
		*/
		bool			releaseObject( OBJECT_CTYPE& obj );
		
		virtual bool	onTimer( unsigned int id, const void* pUserData );

	protected:
		/**
		* ��ʱ��������
		*/
		struct stTempObject
		{
			const time_t	_closeTime;
			OBJECT_CTYPE	_ptrObject;
			
			stTempObject( OBJECT_CTYPE& obj, bool init = false );// : _closeTime( time( nullptr ) - ( init ? CLOSED_SOCKET_MAX_DURATION : 0 ) ), _ptrObject( obj ) {}

			/**
			* �Ƿ񳬹���ʱ
			*/
			bool	isTimeout() const;
			bool	isTimeout( time_t now ) const;
		};

		CONTAINER_TYPE		m_objContainer;
		boost::mutex		m_objMutex;

		typedef container::list<stTempObject>				CONTAINER_TEMP;		
		CONTAINER_TEMP		m_tempContainer;
		boost::mutex		m_tempMutex;
	};


	//---------------------------

	//�����������
#ifndef MAX_OBJECT_NUM
#define MAX_OBJECT_NUM					4096
#endif

	//�Ƿ����ö���
	//����������ã������л���ص���ʱ������Զ�����ͷţ�ֱ��Ԥ��������Զ������ͷţ�������ΪSOCKET_FREE_INTERVAL���룩
#ifndef SOCKET_FREE_INTERVAL
#define SOCKET_FREE_INTERVAL			10
#endif

	//�Ƿ��Զ�����ѹر��׽���
	//�ڶ���������Ƚ��Ӵ������£���������Ӱ������
	//�ڶ����ӣ������ӵ�ϵͳ�£����ʺ�ʹ�ô˹���
	//�Զ�����ļ������ΪCLEAR_CLOSED_SOCKET_INTERVAL(��)
#ifdef AUTO_RECYCLE_SOCKET
#ifndef RECYCLE_SOCKET_INTERVAL
#define RECYCLE_SOCKET_INTERVAL			60
#endif
#endif

	//�������ö���ʱ�������ж��Ƿ�����õĻ���ʱ��
	//δ�������ö���ʱ�����ͷ�ǰ�Ļ���ʱ��
#ifndef CLOSED_SOCKET_MAX_DURATION
#define CLOSED_SOCKET_MAX_DURATION		5
#endif

#define FREE_TIMER_ID					1
#define RECYCLE_TIMER_ID				2

	template<typename OBJECT>
	XAsioPool<typename OBJECT>::stTempObject::stTempObject( OBJECT_CTYPE& obj, bool init ) : _closeTime( time( nullptr ) - ( init ? CLOSED_SOCKET_MAX_DURATION : 0 ) ), _ptrObject( obj ) {}

	template<typename OBJECT>
	bool XAsioPool<typename OBJECT>::stTempObject::isTimeout() const { return isTimeout( time( nullptr ) ); }

	template<typename OBJECT>
	bool XAsioPool<typename OBJECT>::stTempObject::isTimeout( time_t now ) const { return _closeTime <= now - CLOSED_SOCKET_MAX_DURATION; }

	template<typename OBJECT>
	void XAsioPool<typename OBJECT>::start()
	{
		setTimer( FREE_TIMER_ID, 1000 * SOCKET_FREE_INTERVAL, nullptr );
#ifdef AUTO_RECYCLE_SOCKET
		setTimer( RECYCLE_TIMER_ID, 1000 * CLEAR_CLOSED_SOCKET_INTERVAL, nullptr );
#endif
	}

	template<typename OBJECT>
	void XAsioPool<typename OBJECT>::stop()
	{
		stopAllTimer();
	}

	template<typename OBJECT>
	bool XAsioPool<typename OBJECT>::addObject( OBJECT_CTYPE& obj )
	{
		mutex::scoped_lock lock( m_objMutex );
		size_t num = m_objContainer.size();
		if ( num < MAX_OBJECT_NUM )
		{
			m_objContainer.push_back( obj );
		}
		lock.unlock();
		return num < MAX_OBJECT_NUM;
	}

	template<typename OBJECT>
	bool XAsioPool<typename OBJECT>::releaseObject( OBJECT_CTYPE& obj )
	{
		bool bFound = false;

		mutex::scoped_lock lock( m_objMutex );
		CONTAINER_TYPE::iterator it = std::find( std::begin( m_objContainer ), std::end( m_objContainer ), obj );
		if ( it != std::end( m_objContainer ) )
		{
			bFound = true;
			m_objContainer.erase(it);
		}
		lock.unlock();

		if ( bFound )
		{
			mutex::scoped_lock lockTemp( m_tempMutex );
			m_tempContainer.push_back( obj );
		}
		return bFound;
	}

	template<typename OBJECT>
	boost::shared_ptr<OBJECT> XAsioPool<typename OBJECT>::queryObject()
	{
		OBJECT_TYPE obj = reuseObject();
		addObject( obj );
		return obj;
	}

	template<typename OBJECT>
	boost::shared_ptr<OBJECT> XAsioPool<typename OBJECT>::reuseObject()
	{
		mutex::scoped_lock lock( m_tempMutex );
		//��������رջ���ʱ��Ķ�����������
		CONTAINER_TEMP::iterator it;
		for ( it = std::begin( m_tempContainer ); it->isTimeout() && it != std::end( m_tempContainer ); it++ )
		{
			//ʹ�ö����isStarted()�ж��Ƿ�����
			if ( !it->_ptrObject->isStarted() )
			{
				OBJECT_TYPE obj( std::move( it->_ptrObject ) );
				m_tempContainer.erase( it );
				lock.unlock();
				return obj;
			}
		}
		return OBJECT_TYPE( new OBJECT );
	}

	template<typename OBJECT>
	bool XAsioPool<typename OBJECT>::onTimer( unsigned int id, const void* pUserData )
	{
		switch(id)
		{
		case FREE_TIMER_ID:
			freeObjects();
			return true;
			break;
#ifdef AUTO_RECYCLE_SOCKET
		case RECYCLE_TIMER_ID:
			{
				CONTAINER_TYPE objects;
				recycleObject( objects );
				if ( !objects.empty() )
				{
					mutex::scoped_lock lock( m_tempMutex );
					m_tempContainer.insert( std::end( m_tempContainer ), std::begin( objects ), std::end( objects ) );
				}
				return true;
			}
			break;
#endif
		default:
			return XAsioTimer::onTimer( id, pUserData );
			break;
		}
		return false;
	}

	template<typename OBJECT>
	void XAsioPool<typename OBJECT>::allocateObject( int size )
	{
		while( size-- > 0 )
		{
			m_tempContainer.push_back( stTempObject( OBJECT_TYPE( new OBJECT ), true ) );
		}
	}

	template<typename OBJECT>
	size_t XAsioPool<typename OBJECT>::getSize()
	{
		mutex::scoped_lock lock( m_objMutex );
		return m_objContainer.size();
	}

	template<typename OBJECT>
	size_t XAsioPool<typename OBJECT>::getClosedSize()
	{
		mutex::scoped_lock lock( m_tempMutex );
		return m_tempContainer.size();
	}

	template<typename OBJECT>
	boost::shared_ptr<OBJECT> XAsioPool<typename OBJECT>::getAt( size_t index )
	{
		mutex::scoped_lock lock( m_objMutex );
		assert( index < m_objContainer.size() );
		return index < m_objContainer.size() ? *( std::next( std::begin( m_objContainer ), index ) ) : OBJECT_TYPE();
	}
	
	template<typename OBJECT>
	void XAsioPool<typename OBJECT>::recycleObject( CONTAINER_TYPE& objects )
	{
		mutex::scoped_lock lock( m_objMutex );
		for ( CONTAINER_TYPE_ITER it = std::begin( m_objContainer ); it != std::end( m_objContainer ); )
		{
			if ( !(*it)->_ptrObject->isStarted() )
			{
				objects.resize( objects.size() + 1 );
				objects.back().swap(*it);
				it = m_objContainer.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	template<typename OBJECT>
	void XAsioPool<typename OBJECT>::freeObjects( size_t num = -1 )
	{
		if ( 0 == num )
		{
			return;
		}
		mutex::scoped_lock lock( m_tempMutex );
		CONTAINER_TEMP::iterator it;
		for ( it = std::begin( m_tempContainer ); num > 0 && it->isTimeout() && it != std::end( m_tempContainer ); )
		{
			if ( !it->_ptrObject->isStarted() )
			{
				it = m_tempContainer.erase(it);
				num--;
			}
			else
			{
				it++;
			}
		}
	}
}