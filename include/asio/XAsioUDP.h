#pragma once

#include "XAsioInterface.h"

namespace XASIO
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

	class XAsioUDPSession : public XAsioSessionInterface, public boost::enable_shared_from_this<XAsioUDPSession>
	{
	public:
		static UdpSessionPtr	create( XAsioService& io );

	public:
		~XAsioUDPSession();

		virtual void		init();
		virtual void		release();

		virtual void		read();
		virtual void		read( size_t bufferSize );
		virtual void		write( const XAsioBuffer& buffer );

		const UdpSocketPtr&	getSocket() const;

	protected:
		XAsioUDPSession( XAsioService& service );

		UdpSocketPtr		m_socket;

		friend class		XAsioUDPClient;
		friend class		XAsioUDPServer;
	};
	
	class XAsioUDPClient : public XAsioClientInterface, public boost::enable_shared_from_this<XAsioUDPClient>
	{
	public:
		static UdpClientPtr	create( XAsioService& io );

	public:
		~XAsioUDPClient();

		virtual void	init();
		virtual void	release();

		virtual void	connect( const std::string& host, uint16_t port );
		virtual void	connect( const std::string& host, const std::string& protocol );

		template< typename HANDLER, typename OBJECT >
		inline void		setConnectHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { setConnectHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) ); }
		void			setConnectHandler( const std::function< void( UdpSessionPtr ) >& eventHandler );
		
		const UdpResolverPtr& getResolver() const;

	protected:
		XAsioUDPClient( XAsioService& io );

		virtual void	onConnect( UdpSessionPtr session, const boost::system::error_code& err );

		virtual void	onResolve( const boost::system::error_code& err, udp::resolver::iterator it );

	protected:
		UdpResolverPtr		m_resolver;

		std::function< void( UdpSessionPtr ) >	m_funcConnectHandler;
	};

	class XAsioUDPServer : public XAsioServerInterface, public boost::enable_shared_from_this<XAsioUDPServer>
	{
	public:
		static UdpServerPtr	create( XAsioService& io );

	public:
		~XAsioUDPServer();

		virtual void		init();
		virtual void		release();

	public:
		template< typename HANDLER, typename OBJECT >
		inline void		setAcceptHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { setAcceptHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) ); }

		void			setAcceptHandler( const std::function<void( UdpSessionPtr )>& eventHandler );

		virtual void	startAccept( int threadNum, uint16_t port );

	protected:
		XAsioUDPServer( XAsioService& io );

		std::function<void( UdpSessionPtr )>	m_funcAcceptHandler;
	};
}