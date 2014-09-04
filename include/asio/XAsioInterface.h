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
	 * 客户端接口声明
	 */
	class XGAME_API XAsioClientInterface : public XAsioInterface
	{
	protected:
		XAsioClientInterface( XAsioService& service );
	
	public:
		~XAsioClientInterface();

		/**
		 * 连接到指定地址和端口
		 * @param	host	连接的地址
		 * @param	port	连接的端口
		 */
		virtual void	connect( const std::string& host, uint16_t port ) = 0;
		virtual void	connect( const std::string& host, const std::string& protocol ) = 0;
		
	public:
		void			setResolveHandler( std::function<void()> handler );
		
	protected:
		std::function<void()>	m_funcResolveHandler;
	};

	/**
	 * 服务器接口声明
	 */
	class XGAME_API XAsioServerInterface : public XAsioInterface
	{
	public:
		/**
		 * 开始侦听接收
		 */
		virtual void	startAccept( int threadNum, uint16_t port ) = 0;

	protected:
		XAsioServerInterface( XAsioService& service );
	};
}