#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//-------log server
	XLogServer::XLogServer()
	{
	}
	void XLogServer::onProcessMsgPacket( XAsioRecvPacket& recv )
	{
		switch( recv.getHeader()->getType() )
		{
		case EN_S2S_LOG_NFY:
			std::string s;
			recv >> s;
			onLog( outputString( "write log:%s", s.c_str() ) );
			break;
		}
	}
}