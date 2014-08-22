template <typename TYPE>
XTableDataLoader<TYPE>::XTableDataLoader()
{
	m_enFileType = EN_TABLEFILE_CSV;
}

template <typename TYPE>
XTableDataLoader<TYPE>::~XTableDataLoader()
{
	m_mapItems.clear();
}

template <typename TYPE>
bool XTableDataLoader<TYPE>::load()
{
	XCSVParser csvParser;

	if ( !csvParser.parseFile( TYPE::getFileName() ) )
	{
		return false;
	}

	size_t	index = csvParser.getRowByString( "ID" );
	if ( index == -1 )
	{
		return false;
	}

	size_t maxRow = csvParser.getRowCount();
	for ( size_t inx = index + 1; inx < maxRow; inx++ )
	{
		const char* pStr = csvParser.getData( inx, 0 );
		if ( !pStr )
		{
			break;
		}
		TYPE item;
		std::vector<std::string> v;
		csvParser.getRow( inx, v );		
		item.parse( v );
		m_mapItems.insert( std::make_pair( item.getId(), item ) );
	}
	m_mapItemIter = m_mapItems.begin();
	return true;
}

template <typename TYPE>
bool XTableDataLoader<TYPE>::reload()
{
	m_mapItems.clear();
	load();
	return true;
}

template <typename TYPE>
bool XTableDataLoader<TYPE>::clear()
{
	m_mapItems.clear();
	return true;
}

template <typename TYPE>
TYPE * XTableDataLoader<TYPE>::getItem( int id )
{
	MAPITEM::iterator it = m_mapItems.find( id );
	return it == m_mapItems.end() ? NULL : &it->second;
}

template <typename TYPE>
TYPE* XTableDataLoader<TYPE>::getAt( int index )
{
	if ( index < 0 || index >= (int)m_mapItems.size() )
	{
		return NULL;
	}
	MAPITEM::iterator it = m_mapItems.begin();
	while( index > 0 )
	{
		it++
			index--;
	}
	return &it->second;
}

template <typename TYPE>
TYPE* XTableDataLoader<TYPE>::getFirst()
{
	return m_mapItems.empty() ? NULL : &m_mapItems.begin()->second;
}

template <typename TYPE>
TYPE* XTableDataLoader<TYPE>::getNext()
{
	if ( m_mapItemIter != m_mapItems.end() )
	{
		m_mapItemIter++;
		return &m_mapItemIter->second;
	}
	return NULL;
}