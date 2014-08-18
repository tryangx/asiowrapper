#pragma once

#include <string>

#include <boost/asio.hpp>
#include <functional>

#include "XAsioService.h"
#include "XAsioHelper.h"
#include "XAsioPacket.h"

namespace XGAME
{
	/**
	 * 客户端接口声明
	 */
	class XAsioClientInterface : public XAsioInterface
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
		template< typename HANDLER, typename OBJECT >
		void			setResolveHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcResolveHandler = std::bind( eventHandler, eventHandlerObject ); }
		
	protected:
		std::function<void()>	m_funcResolveHandler;
	};


	/**
	 * 服务器接口声明
	 */
	class XAsioServerInterface : public XAsioInterface
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