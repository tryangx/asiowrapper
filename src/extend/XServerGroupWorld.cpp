#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{

	//-------gate server
	XWorldServer::XWorldServer()
	{
	}
	void XWorldServer::onProcessMsgPacket( XAsioRecvPacket& recv )
	{
		switch( recv.getHeader()->getType() )
		{
		case EN_MSG_LOGIN:
			{
				unsigned int id;
				recv >> id;
				onLog( outputString( "World: player [%d] login", id ) );
				XAsioSendPacket send( EN_W2D_LOGIN_REQ );
				send << id;
				sendToServer( EN_APPSERVER_DB, send );
			}			
			break;
		case EN_MSG_LOGOUT:
			{
				unsigned int id;
				recv >> id;
				onLog( outputString( "World: player [%d] logout", id ) );
				XAsioSendPacket send( EN_W2D_LOGOUT_REQ );
				send << id;
				sendToServer( EN_APPSERVER_DB, send );
			}
			break;
		}
	}
}