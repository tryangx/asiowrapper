#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//-------gate server
	XGateServer::XGateServer()
	{
	}

	void XGateServer::onProcessMsgPacket( XAsioRecvPacket& recv )
	{
		try
		{
		switch( recv.getHeader()->getType() )
		{
		case EN_MSG_LOGIN:
			{
				unsigned int id;
				recv >> id;
				XAsioSendPacket send( EN_MSG_LOGIN );
				send.getHeader()->setCmdOp( EN_APPSERVER_WORLD );
				send << id;
				sendToServer( EN_APPSERVER_WORLD, send );
			}
			break;

		case EN_MSG_LOGOUT:
			{
				unsigned int id;
				recv >> id;
				XAsioSendPacket send( EN_MSG_LOGOUT );
				send.getHeader()->setCmdOp( EN_APPSERVER_WORLD );
				send << id;
				sendToServer( EN_APPSERVER_WORLD, send );
			}
			break;
		}
		}
		catch(std::runtime_error& e)
		{
			onLog( e.what() );
		}
	}
}