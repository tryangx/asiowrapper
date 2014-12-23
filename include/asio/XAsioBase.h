#pragma once

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
using namespace boost;
using namespace boost::asio;
using namespace boost::system;

//-------------------------------------------------------------

#define XASIO_WRAPPER_VERSION 10000

#if !defined _MSC_VER && !defined __GNUC__
#error XASIO only support vc and gcc.
#endif

/*
#if defined _MSC_VER && _MSC_VER < 1600
#error XASIO must be compiled with vc2010 or higher.
#endif
*/

//After a roughly reading from gcc.gnu.org, I guess that the minimum version of gcc that support c++0x
//is 4.6, so, I supply the following compiler verification. If there's something wrong, you can freely
//modify them, and if you let me know, I'll be very appreciated.
#if defined __GNUC__ && (__GNUC__ < 4 || __GNUC__ == 4 && __GNUC_MINOR__ < 6)
#error XASIO must be compiled with gcc4.6 or higher.
#endif

//-------------------------------------------------------------

namespace XGAME
{
#ifndef UNIFIED_OUT_BUF_NUM
#define UNIFIED_OUT_BUF_NUM		2048
#endif

#if defined _MSC_VER
#define size_t_format "%Iu"
#define ST_THIS //workaround to make up the BOOST_AUTO's defect under vc2008 and compiler bugs before vc2012
#else // defined __GNUC__
#define size_t_format "%tu"
#define ST_THIS this->
#endif

	//----------------------------------------------
#define ON_CALLBACK( FUNC_PTR )							if ( FUNC_PTR != nullptr ) { FUNC_PTR(); }
#define ON_CALLBACK_PARAM( FUNC_PTR, PARAM )			if ( FUNC_PTR != nullptr ) { FUNC_PTR( PARAM ); }
#define ON_CALLBACK_PARAM2( FUNC_PTR, PARAM1, PARAM2 )	if ( FUNC_PTR != nullptr ) { FUNC_PTR( PARAM1, PARAM2 ); }

	//----------------------------------------------
	//	共享对象宏
#define SHARED_OBJECT(CLASS_NAME, FATHER_NAME) \
	class CLASS_NAME : public FATHER_NAME, public boost::enable_shared_from_this<CLASS_NAME>

#define SHARED_OBJECT_T(CLASS_NAME, FATHER_NAME, TYPENAME) \
	class CLASS_NAME : public FATHER_NAME, public boost::enable_shared_from_this<CLASS_NAME<TYPENAME>>
	
	//----------------------------------------------
	//	遍历宏
#if !defined _MSC_VER || _MSC_VER >= 1700
	template<typename CONTAINER, typename MUTEX, typename PREDICATE>
	void forEachAll( CONTAINER& _con, MUTEX& _mutex, const PREDICATE& _pred )
	{ mutex::scoped_lock lock( _mutex ); for ( auto& item : _con ) _pred( item ); }

	template<typename CONTAINER, typename PREDICATE>
	void forEachAll( CONTAINER& _con, const PREDICATE& _pred ) { for ( auto& item : _con ) _pred( item ); }
#else
	template<typename CONTAINER, typename MUTEX, typename PREDICATE>
	void forEachAll( CONTAINER& _con, MUTEX& _mutex, const PREDICATE& _pred )
	{ mutex::scoped_lock lock( _mutex ); std::for_each( std::begin( _con ), std::end( _con ), [&]( decltype(*std::begin(_con))& item ) { _pred( item ); } ); }

	template<typename CONTAINER, typename PREDICATE>
	void forEachAll( CONTAINER& _con, const PREDICATE& _pred )
	{ std::for_each( std::begin( _con ), std::end( _con ), [&]( decltype(*std::begin(_con))& item ) { _pred( item ); } ); }
#endif

	//member functions, used to do something to any member container optionally with any member mutex
#define FOREACH_ALL_MUTEX( CONTAINER, MUTEX )	FOREACH_ALL_MUTEX_NAME( forEachAll, CONTAINER, MUTEX )
#define FOREACH_ALL( CONTAINER )				FOREACH_ALL_NAME( forEachAll, CONTAINER )


#if !defined _MSC_VER || _MSC_VER >= 1700
#define FOREACH_ALL_MUTEX_NAME( FUNNAME, CONTAINER, MUTEX ) \
	template<typename PREDICATE>\
	void FUNNAME( const PREDICATE& _pred ) \
	{ mutex::scoped_lock lock( MUTEX ); for ( auto& item : CONTAINER ) _pred( item ); }
#define FOREACH_ALL_NAME( FUNNAME, CONTAINER ) \
	template<typename PREDICATE>\
	void FUNNAME( const PREDICATE& _pred ) \
	{ for ( auto& item : CONTAINER ) _pred( item );} \
	template<typename PREDICATE>\
	void FUNNAME( const PREDICATE& _pred ) const \
	{ for ( auto& item : CONTAINER ) _pred( item ); }
#else
#define FOREACH_ALL_MUTEX_NAME( FUNNAME, CONTAINER, MUTEX ) \
	template<typename PREDICATE> void FUNNAME( const PREDICATE& _pred ) \
	{ mutex::scoped_lock lock( MUTEX ); std::for_each( std::begin(CONTAINER), std::end(CONTAINER), [&](decltype(*std::begin(CONTAINER))& item) { _pred( item ); }); }
#define FOREACH_ALL_NAME( FUNNAME, CONTAINER ) \
	template<typename PREDICATE> void FUNNAME( const PREDICATE& _red ) \
	{std::for_each( std::begin(CONTAINER), std::end(CONTAINER), [&](decltype(*std::begin( CONTAINER))& item ) { _pred(item); }); } \
	template<typename PREDICATE> void FUNNAME( const PREDICATE& _pred ) const \
	{std::for_each( std::begin(CONTAINER), std::end(CONTAINER), [&](decltype(*std::begin(CONTAINER))& item) { _pred(item); }); }
#endif

	//------------------------------
	// for every if predicate returns true

	template<typename CONTAINER, typename MUTEX, typename PREDICATE>
	void forEveryAll( CONTAINER& _con, MUTEX& _mutex, const PREDICATE& _pred )
	{ mutex::scoped_lock lock( mutex ); for ( auto it = std::begin( _con ); it != std::end( _con ); it++ ) if ( _pred( *it ) ) break; }

	template<typename CONTAINER, typename PREDICATE>
	void forEveryOne( CONTAINER& _con, const PREDICATE& _pred )
	{ for ( auto it = std::begin( _con ); it != std::end( _con ); it++ ) if ( _pred( *it ) ) break; }


#define FOREVERY_ONE_MUTEX(CONTAINER, MUTEX) FOREVERY_ONE_MUTEX_NAME( forEveryAll, CONTAINER, MUTEX )
#define FOREVERY_ONE(CONTAINER) FOREVERY_ONE_NAME( forEveryAll, CONTAINER )

#define FOREVERY_ONE_MUTEX_NAME( FUNNAME, CONTAINER, MUTEX ) \
	template<typename PREDICATE> void FUNNAME(const PREDICATE& _pred) \
	{ mutex::scoped_lock lock( MUTEX ); for ( auto it = std::begin( CONTAINER ); it != std::end( CONTAINER ); it++ ) if ( _pred( *it ) ) break; }
#define FOREVERY_ONE_NAME( FUNNAME, CONTAINER ) \
	template<typename PREDICATE> void FUNNAME( const PREDICATE& _pred ) \
	{ for ( auto it = std::begin( CONTAINER ); it != std::end( CONTAINER ); it++ ) if ( _pred( *it ) ) break; } \
	template<typename PREDICATE> void FUNNAME( const PREDICATE& _pred ) const \
	{ for ( auto it = std::begin( CONTAINER ); it != std::end( CONTAINER ); it++ ) if ( _pred( *it ) ) break; }

	//-------------------------------------------------------------------
	// 合并容器
	template<typename CONTAINER>
	bool spliceContainer( CONTAINER& destContainer, CONTAINER& sourContainer, size_t maxSize = MAX_MSG_NUM )
	{
		auto size = destContainer.size();
		if (size < maxSize) //dest_can's buffer available
		{
			size = maxSize - size; //max items this time can handle
			auto begin_iter = std::begin(sourContainer), end_iter = std::end(sourContainer);
			if (sourContainer.size() > size) //some items left behind
			{
				auto left_num = sourContainer.size() - size;
				//find the minimum movement
				end_iter = left_num > size ? std::next(begin_iter, size) : std::prev(end_iter, left_num);
			}
			else
			{
				size = sourContainer.size();
			}
			//use size to avoid std::distance() call, so, size must correct
			destContainer.splice(std::end(destContainer), sourContainer, begin_iter, end_iter, size);

			return size > 0;
		}
		return false;
	}
}