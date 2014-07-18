#include "../../include/asio/XAsioInterface.h"
#include "../../include/asio/XAsioService.h"

namespace XASIO
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
	
	//-------------------------------------------
	//	����˽ӿ�ʵ��

	XAsioServerInterface::XAsioServerInterface( XAsioService& service )
		: XAsioInterface( service )
	{
	}
}