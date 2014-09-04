#pragma once

#include "XAsioService.h"
#include "XAsioHelper.h"
#include "XAsioPacket.h"

#include <boost/asio.hpp>
#include <functional>
#include <string>

namespace XGAME
{
	/**
	 * �ͻ��˽ӿ�����
	 */
	class XGAME_API XAsioClientInterface : public XAsioInterface
	{
	protected:
		XAsioClientInterface( XAsioService& service );
	
	public:
		~XAsioClientInterface();

		/**
		 * ���ӵ�ָ����ַ�Ͷ˿�
		 * @param	host	���ӵĵ�ַ
		 * @param	port	���ӵĶ˿�
		 */
		virtual void	connect( const std::string& host, uint16_t port ) = 0;
		virtual void	connect( const std::string& host, const std::string& protocol ) = 0;
		
	public:
		void			setResolveHandler( std::function<void()> handler );
		
	protected:
		std::function<void()>	m_funcResolveHandler;
	};

	/**
	 * �������ӿ�����
	 */
	class XGAME_API XAsioServerInterface : public XAsioInterface
	{
	public:
		/**
		 * ��ʼ��������
		 */
		virtual void	startAccept( int threadNum, uint16_t port ) = 0;

	protected:
		XAsioServerInterface( XAsioService& service );
	};
}