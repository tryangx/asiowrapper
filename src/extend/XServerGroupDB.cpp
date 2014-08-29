#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{

	//-------db server
	XDBServer::XDBServer()
	{
	}
	void XDBServer::onProcessMsgPacket( XAsioRecvPacket& recv )
	{
		switch( recv.getHeader()->getType() )
		{
		case EN_MSG_LOGIN:
			{
				unsigned int id;
				recv >> id;
				onLog( outputString( "DB: recv [%d] login", id ) );
				XAsioSendPacket send( EN_D2W_LOGIN_RES );
				send.getHeader()->setCmdOp( EN_APPSERVER_WORLD );
				send << int( 12345 );
				send << std::string( "playername" );
				send << short( 99 );
				sendToServer( EN_APPSERVER_WORLD, send );
			}
			break;
		case EN_MSG_LOGOUT:
			{
				unsigned int id;
				recv >> id;
				onLog( outputString( "DB: recv [%d] logout", id ) );
				XAsioSendPacket send( EN_D2W_LOGIN_RES );
				send.getHeader()->setCmdOp( EN_APPSERVER_WORLD );
				sendToServer( EN_APPSERVER_CENTER, send );
			}
			break;
		}
	}
}