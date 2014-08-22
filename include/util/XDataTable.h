/**
 * 数据表管理
 */
#pragma once
#include "XApi.h"
#include "XSingleton.h"
#include "XCSVParser.h"
#include <map>

namespace XGAME
{
	enum enTableDataFileType
	{
		EN_TABLEFILE_CSV	=	1,
		EN_TABLEFILE_XML	=	2,
	};

	/**
	 * TYPE需要支持下列接口
	 * 1.static const char*	getFileName();
	 * 2.void			parse( std::vector<std::string> )
	 * 3.int			getId()
	 */
	template<typename TYPE>
	class XGAME_API XTableDataLoader
	{
	public:
		XTableDataLoader();
		~XTableDataLoader();

		bool	load();
		bool	reload();
		bool	clear();

		TYPE*	getItem( int id );

		int		getItemCount();

		TYPE*	getAt( int index );
		TYPE*	getFirst();
		TYPE*	getNext();

	protected:
		typedef std::map<int, TYPE>		MAPITEM;
		MAPITEM							m_mapItems;
		typename MAPITEM::iterator		m_mapItemIter;
		enTableDataFileType				m_enFileType;
	};

#include "XDataTable.hpp"
}