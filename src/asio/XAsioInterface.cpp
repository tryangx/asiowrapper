#include "asio/XAsioInterface.h"
#include "asio/XAsioService.h"

namespace XGAME
{
	//-------------------------------------------
	//	客户端接口实现
	
	XAsioClientInterface::XAsioClientInterface( XAsioService& service )
		: XAsioInterface( service ), m_funcResolveHandler( nullptr )		
	{
	}

	XAsioClientInterface::~XAsioClientInterface()
	{
		m_funcResolveHandler = nullptr;
	}
	
	//-------------------------------------------
	//	服务端接口实现

	XAsioServerInterface::XAsioServerInterface( XAsioService& service )
		: XAsioInterface( service )
	{
	}
}