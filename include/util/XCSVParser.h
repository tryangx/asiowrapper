/**
 * CSV parser
 */
#pragma once

#include "XApi.h"
#include <vector>
#include <string>

namespace XGAME
{
	class XGAME_API XCSVParser 
	{
	public:
		XCSVParser();
		~XCSVParser();

	public: 
		bool		parseFile(const char* pFileName );
		
		/**
		 * 得到行数
		 */
		size_t		getRowCount();

		/**
		 * 得到列数
		 */
		size_t		getColumnCount();

		/**
		 * 得到一行的数据
		 */
		void		getRow( size_t row, std::vector<std::string>& v );

		/**
		 * 得到指定位置的数据
		 * @param	row		行
		 * @param	column	列
		 */
		const char*	getData( size_t row, size_t column );

		/**
		 * 查找指定字符串所在列
		 * @param	row		是否指定行，默认则从第一行开始查询
		 * @return 查找不到则返回-1，从0开始
		 */
		size_t		getColumnByString( const char* pName, size_t row = -1 );
		/**
		 * 查找指定字符串所在行
		 * @param	column	是否指定列，默认则从第一行开始查询
		 * @return 查找不到则返回-1，从0开始
		 */
		size_t		getRowByString( const char* pName, size_t column = -1 );

	private:
		void		split( std::vector<std::string>& field, std::string line );
		int			advplain( const std::string& line, std::string& fld, size_t );
		int			advquoted( const std::string& line, std::string& fld, size_t );

	private:
		std::string								m_sFieldSep;
		std::vector<std::vector<std::string>>	m_vLines;
		size_t									m_iMaxColumn;
	};
}