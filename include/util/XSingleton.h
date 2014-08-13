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

	//�ڱ��࣬������̲߳������Զ���������
	//�ڱ��಻������
	template<typename Type>
	class XSingletonGuard : boost::mutex::scoped_lock, public boost::noncopyable
	{
	public:
		explicit XSingletonGuard ( Type* _ptr, boost::mutex& _mutex ) : boost::mutex::scoped_lock(_mutex), m_ptr(_ptr) {}

		Type* operator->() { return m_ptr; }

	private:
		Type*	m_ptr;
	};

	//�����࣬���ڼ��ӵ�����״̬
	template<typename Type>
	class XSingletonWrapper : public Type
	{
	public:
		~XSingletonWrapper() { m_bIsDestroyed = true; }

		static	bool		m_bIsDestroyed;
	};
	template<typename Type>
	bool XSingletonWrapper<Type>::m_bIsDestroyed = false;

	//����
	template<typename Type>
	class XSingleton : public XSingletonModule
	{
	public:
		/**
		 * �õ��ױ�ʵ��(�̰߳�ȫ)
		 */
		static XSingletonGuard<Type> getMutableInstance() { return XSingletonGuard<Type>( &getIntance(), m_mutex ); }
		
		/**
		 * �õ�����ʵ��
		 */
		static const Type& getConstInstance() { return getIntance(); }

		/**
		 * �õ�ʵ��(���̰߳�ȫ)
		 * ��ͨ��lock(),unlock()�ֶ��ӽ���
		 */
		static Type& getUnsafeInstance() { return getInstance(); }
		
		/**
		 * �����Ƿ�����
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