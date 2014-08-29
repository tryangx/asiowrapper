#include "asio/XAsioInterface.h"
#include "asio/XAsioService.h"

namespace XGAME
{
	//-------------------------------------------
	//	�ͻ��˽ӿ�ʵ��
	
	XAsioClientInterface::XAsioClientInterface( XAsioService& service )
		: XAsioInterface( service ), m_funcResolveHandler( nullptr )		
	{
	}

	XAsioClientInterface::~XAsioClientInterface()
	{
		m_funcResolveHandler = nullptr;
	}
	
	void XAsioClientInterface::setResolveHandler( std::function<void()> handler )
	{
		m_funcResolveHandler = handler;
	}

	//-------------------------------------------
	//	����˽ӿ�ʵ��

	XAsioServerInterface::XAsioServerInterface( XAsioService& service )
		: XAsioInterface( service )
	{
	}
}