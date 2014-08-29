#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	//-------center server
	XCenterServer::XCenterServer()
	{

	}
	void XCenterServer::onProcessMsgPacket( XAsioRecvPacket& recv )
	{
		onLog( "center only redirect!" );
	}
}