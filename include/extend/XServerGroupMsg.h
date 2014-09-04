#pragma once

namespace XGAME
{
	//������c2s/s2cЭ��
	enum enServerMsgTestType
	{
		EN_MSG_BEGIN		= 1000,
		
		//msg >> uint id
		EN_MSG_LOGIN,

		//msg << uint id
		EN_MSG_LOGOUT,
	};

	//������s2s����
	enum enServerCmdTestType
	{
		//msg << string content
		EN_CMD_BEGIN		= 10000,
		
		//msg << int id
		EN_CMD_W2D_LOGIN_REQ,
		//msg >> uint id >> string >> short lv
		EN_CMD_D2W_LOGIN_RES,

		EN_CMD_W2D_LOGOUT_REQ,
		EN_CMD_D2W_LOGOUT_RES,

		//msg << string
		EN_CMD_S2S_LOG_NFY,
	};

	enum enServerRegTestType
	{
		//ע��Ӧ�÷�����
		//msg << int type << int id
		//type ����
		//id ���
		EN_REG_APPSERVER,
	};
}