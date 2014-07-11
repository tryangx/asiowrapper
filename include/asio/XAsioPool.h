#pragma once

#include "XAsioBase.h"

#include <boost/container/list.hpp>

namespace XASIO
{
	//---------------------------
	// 对象池

	/**
	* 
	* 模板参数OBJECT需要包含以下接口
	*		isStarted() //是否运行
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
		 * 分配对象
		 */
		void			allocateObject( int size );

		/**
		* 获取数量
		*/
		size_t			getSize();

		/**
		* 获取关闭的数量
		*/
		size_t			getClosedSize();

		/**
		* 获得指定位置的对象
		*/
		OBJECT_TYPE		getAt( size_t index );
		
		/**
		* 清除全部已关闭的对象
		* @param objects 对象容器，满足条件被移除的对象将放置到该容器中
		*/
		void			recycleObject( CONTAINER_TYPE& objects );

		/**
		* 释放指定数量的对象
		* @param num 数量为-1则释放全部
		*/
		void			freeObjects( size_t num = -1 );

		FOREACH_ALL_MUTEX( m_objContainer, m_objMutex );

		FOREVERY_ONE_MUTEX( m_objContainer, m_objMutex );

	protected:
		XAsioPool( io_service& service ) : XAsioTimer( service ) {}

		/**
		* 开启对象池
		* 用于初始化对象重用，自动清除关闭对象等功能
		*/
		void			start();
		void			stop();

		/**
		 * 获取对象
		 */
		OBJECT_TYPE		queryObject();

		/**
		* 重用对象
		*/
		OBJECT_TYPE		reuseObject();

		/**
		* 填加对象
		*/
		bool			addObject( OBJECT_CTYPE& obj );

		/**
		* 返回对象
		*/
		bool			releaseObject( OBJECT_CTYPE& obj );
		
		virtual bool	onTimer( unsigned int id, const void* pUserData );

	protected:
		/**
		* 临时对象数据
		*/
		struct stTempObject
		{
			const time_t	_closeTime;
			OBJECT_CTYPE	_ptrObject;
			
			stTempObject( OBJECT_CTYPE& obj, bool init = false );// : _closeTime( time( nullptr ) - ( init ? CLOSED_SOCKET_MAX_DURATION : 0 ) ), _ptrObject( obj ) {}

			/**
			* 是否超过计时
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

	//最大对象池数量
#ifndef MAX_OBJECT_NUM
#define MAX_OBJECT_NUM					4096
#endif

	//是否重用对象
	//如果开启重用，则所有缓存池的临时对象将永远不被释放，直到预设的周期自动进行释放，此周期为SOCKET_FREE_INTERVAL（秒）
#ifndef SOCKET_FREE_INTERVAL
#define SOCKET_FREE_INTERVAL			10
#endif

	//是否自动清除已关闭套接字
	//在对象池数量比较庞大的情况下，将会严重影响性能
	//在短连接，少连接的系统下，才适合使用此功能
	//自动清除的间隔周期为CLEAR_CLOSED_SOCKET_INTERVAL(秒)
#ifdef AUTO_RECYCLE_SOCKET
#ifndef RECYCLE_SOCKET_INTERVAL
#define RECYCLE_SOCKET_INTERVAL			60
#endif
#endif

	//开启重用对象时，用于判断是否可重用的缓冲时间
	//未开启重用对象时，在释放前的缓冲时间
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
		//查找满足关闭缓冲时间的对象用于重用
		CONTAINER_TEMP::iterator it;
		for ( it = std::begin( m_tempContainer ); it->isTimeout() && it != std::end( m_tempContainer ); it++ )
		{
			//使用对象的isStarted()判断是否运行
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