#pragma once

namespace XGAME
{
	//测试用c2s/s2c协议
	enum enServerMsgTestType
	{
		EN_MSG_BEGIN		= 1000,
		
		//msg >> uint id
		EN_MSG_LOGIN,

		//msg << uint id
		EN_MSG_LOGOUT,
	};

	//测试用s2s命令
	enum enServerCmdTestType
	{
		//msg << string content
		EN_CMD_BEGIN		= 10000,
		
		//msg << int id
		EN_W2D_LOGIN_REQ,
		//msg >> uint id >> string >> short lv
		EN_D2W_LOGIN_RES,

		EN_W2D_LOGOUT_REQ,
		EN_D2W_LOGOUT_RES,

		//msg << string
		EN_S2S_LOG_NFY,
	};
}