#pragma once

#include "XAsioInterface.h"
#include "XAsioSession.h"

namespace XGAME
{
#define	DEFAULT_READ_BYTES		512

	using namespace boost::asio::ip;

	typedef boost::shared_ptr<class XAsioUDPSession>			UdpSessionPtr;
	typedef boost::shared_ptr<udp::socket>						UdpSocketPtr;

	typedef boost::shared_ptr<class XAsioUDPClient>				UdpClientPtr;
	typedef boost::shared_ptr<udp::resolver>					UdpResolverPtr;

	typedef boost::shared_ptr<class XAsioUDPServer>				UdpServerPtr;

	class XAsioUDPClient;
	class XAsioUDPServer;

	class XGAME_API XAsioUDPSession : public XAsioSession, public boost::enable_shared_from_this<XAsioUDPSession>
	{
	public:
		XAsioUDPSession( XAsioServiceController& controller );
		~XAsioUDPSession();

		virtual void		recv();
		virtual void		recv( size_t bufferSize );
		virtual void		send( XAsioBuffer& buffer );

		const UdpSocketPtr&	getSocket() const;

	protected:
		UdpSocketPtr		m_socket;
	};
	
	class XGAME_API XAsioUDPClient : public XAsioClientInterface, public boost::enable_shared_from_this<XAsioUDPClient>
	{
	public:
		~XAsioUDPClient();

		virtual void	init();
		virtual void	release();

		virtual void	connect( const std::string& host, uint16_t port );
		virtual void	connect( const std::string& host, const std::string& protocol );

		template< typename HANDLER, typename OBJECT >
		inline void		setConnectHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { setConnectHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) ); }
		void			setConnectHandler( const std::function< void( UdpSessionPtr ) >& eventHandler );

	protected:
		XAsioUDPClient( XAsioServiceController& controller );

		virtual void	onResolveCallback( const boost::system::error_code& err, udp::resolver::iterator it );
		virtual void	onConnectCallback( UdpSessionPtr session, const boost::system::error_code& err );

	protected:
		UdpResolverPtr		m_ptrResolver;

		std::function< void( UdpSessionPtr ) >	m_funcConnectHandler;
	};

	class XGAME_API XAsioUDPServer : public XAsioServerInterface, public boost::enable_shared_from_this<XAsioUDPServer>
	{
	public:
		~XAsioUDPServer();

		virtual void		init();
		virtual void		release();

	public:
		void			setAcceptHandler( const std::function<void( UdpSessionPtr )>& eventHandler );

		virtual void	startAccept( int threadNum, uint16_t port );

	protected:
		XAsioUDPServer( XAsioServiceController& controller );

		UdpSessionPtr	m_ptrSession;

		std::function<void( UdpSessionPtr )>	m_funcAcceptHandler;
	};
}