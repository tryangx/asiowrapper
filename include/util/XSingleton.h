#pragma once

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/serialization/singleton.hpp>

namespace XGAME
{
	class XSingletonModule : public boost::noncopyable
	{
	public:
		static void lock()		{ getLock() = true; }
		static void unlock()	{ getLock() = false; }
		static bool isLocked()	{ return getLock(); }

	private:
		static bool& getLock()	{ static bool s_lock = false; return s_lock; }
	};

	//哨兵类，负责多线程操作，自动加锁解锁
	//哨兵类不允许拷贝
	template<typename Type>
	class XSingletonGuard : boost::mutex::scoped_lock, public boost::noncopyable
	{
	public:
		explicit XSingletonGuard ( Type* _ptr, boost::mutex& _mutex ) : boost::mutex::scoped_lock(_mutex), m_ptr(_ptr) {}

		Type* operator->() { return m_ptr; }

	private:
		Type*	m_ptr;
	};

	//监视类，用于监视单例的状态
	template<typename Type>
	class XSingletonWrapper : public Type
	{
	public:
		~XSingletonWrapper() { m_bIsDestroyed = true; }

		static	bool		m_bIsDestroyed;
	};
	template<typename Type>
	bool XSingletonWrapper<Type>::m_bIsDestroyed = false;

	//单例
	template<typename Type>
	class XSingleton : public XSingletonModule
	{
	public:
		/**
		 * 得到易变实例(线程安全)
		 */
		static XSingletonGuard<Type> getMutableInstance() { return XSingletonGuard<Type>( &getIntance(), m_mutex ); }
		
		/**
		 * 得到常量实例
		 */
		static const Type& getConstInstance() { return getIntance(); }

		/**
		 * 得到实例(非线程安全)
		 * 可通过lock(),unlock()手动加解锁
		 */
		static Type& getUnsafeInstance() { return getInstance(); }
		
		/**
		 * 对象是否消毁
		 */
		static bool isDestroyed() { return XSingletonWrapper<Type>::m_bIsDestroyed; }

	protected:
		boost::mutex::scoped_lock getScopedLock() { return boost::mutex::scoped_lock( m_mutex ); }
		
		static Type& getIntance()
		{
			static XSingletonWrapper<Type> s_type;
			BOOST_ASSERT( !XSingletonWrapper<Type>::m_bIsDestroyed );
			use( m_instance );
			return static_cast<Type&>( s_type );
		}
	private:
		static void use( Type const & ) {}
				
		static boost::mutex		m_mutex;

		static Type&			m_instance;
	};

	template<typename Type>
	boost::mutex XSingleton<Type>::m_mutex; 

	template<typename Type>
	Type& XSingleton<Type>::m_instance = XSingleton<Type>::getIntance();
}