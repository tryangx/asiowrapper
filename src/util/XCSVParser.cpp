/**
 * 
 *
 */
#include "util/XCSVParser.h"
#include <iostream>
#include <fstream>

namespace XGAME
{
	using namespace std;

	XCSVParser::XCSVParser() : m_iMaxColumn( 0 ), m_sFieldSep( "," )
	{
	}

	XCSVParser::~XCSVParser()
	{
		for ( size_t i=0; i < m_vLines.size(); i++ )
		{
			m_vLines[i].clear();
		}
		m_vLines.clear();
	}

	size_t XCSVParser::getRowCount()
	{
		return m_vLines.size();
	}
	size_t XCSVParser::getColumnCount()
	{
		return m_iMaxColumn;
	}

	void XCSVParser::getRow( size_t row, std::vector<std::string>& v )
	{
		if ( row >= 0 && row < m_vLines.size() )
		{
			v.assign( m_vLines[row].begin(), m_vLines[row].end() );
		}
	}
	
	// getfield: return n-th field
	const char* XCSVParser::getData( size_t row, size_t column )
	{
		if ( row < 0 || row >= m_vLines.size() || column < 0 || column >= m_vLines[row].size() )
		{
			return "";
		}
		return m_vLines[row][column].c_str();
	}

	size_t XCSVParser::getColumnByString( const char* pName, size_t row /* = -1 */ )
	{
		size_t beg = row == -1 ? 0 : row;
		size_t end = row == -1 ? m_vLines.size() : beg + 1;
		for ( size_t i = beg; i < end; i++ )
		{
			vector<string>& vRow = m_vLines[i];
			for ( size_t column = 0; column < vRow.size(); column++ )
			{
				if ( strcmp( pName, vRow[column].c_str() ) == 0 )
				{
					return column;
				}
			}
		}
		return -1;
	}
	size_t XCSVParser::getRowByString( const char* pName, size_t column /* = -1 */ )
	{
		size_t beg = column == -1 ? 0 : column;
		for ( size_t i = 0; i < m_vLines.size(); i++ )
		{
			vector<string>& vRow = m_vLines[i];
			size_t end = column == -1 ? vRow.size() : beg + 1;
			for ( size_t j = beg; j < vRow.size() && j < end; j++ )
			{
				if ( strcmp( pName, vRow[j].c_str() ) == 0 )
				{
					return i;
				}
			}
		}
		return -1;
	}

	bool XCSVParser::parseFile( const char* pFileName )
	{
		ifstream fin( pFileName );
		m_iMaxColumn = 0;
		string s; 
		while( getline( fin,s ) )
		{
			vector<string> field;
			split( field,s );
			m_iMaxColumn = m_iMaxColumn < field.size() ? field.size() : m_iMaxColumn;
			m_vLines.push_back(field);
		}
		return true;
	}

	// split: split line into fields
	void XCSVParser::split( std::vector<string>& field, std::string line )
	{
		string fld;
		size_t i, j;

		if (line.length() == 0)
			return ;
		i = 0;

		do 
		{
			if (i < line.length() && line[i] == '"')
				j = advquoted(line, fld, ++i);    // skip quote
			else
				j = advplain(line, fld, i);

			field.push_back(fld);
			i = j + 1;
		} while (j < line.length());

	}

	// advquoted: quoted field; return index of next separator
	int XCSVParser::advquoted( const std::string& s, std::string& fld, size_t i )
	{
		size_t j;

		fld = "";
		for (j = i; j < s.length(); j++)
		{
			if (s[j] == '"' && s[++j] != '"')
			{
				size_t k = s.find_first_of( m_sFieldSep, j );
				if (k > s.length())    // no separator found
					k = s.length();
				for (k -= j; k-- > 0; )
					fld += s[j++];
				break;
			}
			fld += s[j];
		}
		return j;
	}

	// advplain: unquoted field; return index of next separator
	int XCSVParser::advplain( const std::string& s, std::string& fld, size_t i )
	{
		size_t j;

		j = s.find_first_of( m_sFieldSep, i ); // look for separator
		if (j > s.length())               // none found
			j = s.length();
		fld = string(s, i, j-i);
		return j;
	}
}