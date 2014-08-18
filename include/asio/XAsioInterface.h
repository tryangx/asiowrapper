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
	 * �ͻ��˽ӿ�����
	 */
	class XAsioClientInterface : public XAsioInterface
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
		template< typename HANDLER, typename OBJECT >
		void			setResolveHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcResolveHandler = std::bind( eventHandler, eventHandlerObject ); }
		
	protected:
		std::function<void()>	m_funcResolveHandler;
	};


	/**
	 * �������ӿ�����
	 */
	class XAsioServerInterface : public XAsioInterface
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