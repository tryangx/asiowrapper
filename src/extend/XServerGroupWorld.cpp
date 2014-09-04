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
				onLog( outputString( "player [%d] login", id ) );
				XAsioSendPacket send( EN_CMD_W2D_LOGIN_REQ );
				send << id;
				sendToServer( EN_APPSERVER_DB, send );
			}			
			break;
		case EN_MSG_LOGOUT:
			{
				unsigned int id;
				recv >> id;
				onLog( outputString( "player [%d] logout", id ) );
				XAsioSendPacket send( EN_CMD_W2D_LOGOUT_REQ );
				send << id;
				sendToServer( EN_APPSERVER_DB, send );
			}
			break;
	
		case EN_CMD_D2W_LOGIN_RES:
			{
				unsigned int pid;
				std::string s;
				short lv;
				recv >> pid >> s >> lv;
				onLog( outputString( "pid=%d name=%s lv=%d", pid, s.c_str(), lv ) );
			}
			break;
		case EN_CMD_D2W_LOGOUT_RES:
			{
				unsigned int pid;
				recv >> pid;
				onLog( outputString( "logout %d", pid ) );
			}
			break;
		}
	}
}