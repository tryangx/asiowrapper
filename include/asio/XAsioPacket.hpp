template<typename TYPE>
XAsioBuffer& XAsioBuffer::operator << ( const TYPE &value )
{
	if ( m_bufData._dataSize + sizeof(TYPE) > MAX_PACKET_SIZE )
	{
		return *this;
	}
	resize( m_bufData._dataSize + sizeof(TYPE) );
	try
	{
		char* pCur = (char*)m_bufData._pData + m_bufData._dataSize;
		*(TYPE*)pCur = value;
	}
	catch(...) {}
	m_bufData._dataSize += sizeof(TYPE);
	return *this;
}
template<typename TYPE>
XAsioBuffer& XAsioBuffer::operator >> ( TYPE &value )
{
	if ( getRemainSize() >= sizeof(TYPE) )
	{
		char* pCur = (char*)m_bufData._pData + m_iCursorPos;
		value = *(TYPE*)pCur;
		m_iCursorPos += sizeof(TYPE);
	}
	return *this;
}

template <typename TYPE>
XAsioSendPacket& XAsioSendPacket::operator << ( const TYPE &value )
{
	if ( getCurPos() + sizeof(TYPE) <= MAX_PACKET_SIZE )
	{
		*(TYPE*)m_pCurPtr = value;
		m_pHeader->m_dwSize += sizeof(TYPE);
		m_pCurPtr += sizeof(TYPE);
	}
	return *this;
}

template <typename TYPE>
XAsioRecvPacket& XAsioRecvPacket::operator >> ( TYPE &value )
{
	if ( getRemainSize() >= sizeof(TYPE) )
	{
		value = *(TYPE*)m_pCurPtr;
		m_pCurPtr += sizeof(TYPE);
	}
	return *this;
}