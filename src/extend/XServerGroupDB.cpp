#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{

	//-------db server
	XDBServer::XDBServer()
	{
		m_iCounter = 0;
	}

	void XDBServer::onProcessMsgPacket( XAsioRecvPacket& recv )
	{
		switch( recv.getHeader()->getType() )
		{
		case EN_CMD_W2D_LOGIN_REQ:
			{
				m_iCounter++;
				unsigned int id;
				recv >> id;
				onLog( outputString( "recv [%d] login", id ) );
				XAsioSendPacket send( EN_CMD_D2W_LOGIN_RES );
				send.getHeader()->setCmdOp( EN_APPSERVER_WORLD );
				send << int( m_iCounter );
				send << std::string( "name" );
				send << short( 99 );
				sendToServer( EN_APPSERVER_WORLD, send );
			}
			break;
		case EN_CMD_W2D_LOGOUT_REQ:
			{
				unsigned int id;
				recv >> id;
				onLog( outputString( "recv [%d] logout", id ) );
				XAsioSendPacket send( EN_CMD_D2W_LOGOUT_RES );
				send.getHeader()->setCmdOp( EN_APPSERVER_WORLD );
				sendToServer( EN_APPSERVER_CENTER, send );				
			}
			break;
		}
	}
}