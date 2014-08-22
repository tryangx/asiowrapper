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
		 * �õ�����
		 */
		size_t		getRowCount();

		/**
		 * �õ�����
		 */
		size_t		getColumnCount();

		/**
		 * �õ�һ�е�����
		 */
		void		getRow( size_t row, std::vector<std::string>& v );

		/**
		 * �õ�ָ��λ�õ�����
		 * @param	row		��
		 * @param	column	��
		 */
		const char*	getData( size_t row, size_t column );

		/**
		 * ����ָ���ַ���������
		 * @param	row		�Ƿ�ָ���У�Ĭ����ӵ�һ�п�ʼ��ѯ
		 * @return ���Ҳ����򷵻�-1����0��ʼ
		 */
		size_t		getColumnByString( const char* pName, size_t row = -1 );
		/**
		 * ����ָ���ַ���������
		 * @param	column	�Ƿ�ָ���У�Ĭ����ӵ�һ�п�ʼ��ѯ
		 * @return ���Ҳ����򷵻�-1����0��ʼ
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