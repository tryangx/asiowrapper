#pragma once

#include "XAsioInterface.h"
#include "XAsioSession.h"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

namespace XGAME
{
	enum enBufferType
	{
		EN_SESSION_SEND_BUFFER,
		EN_SESSION_RECV_BUFFER,

		EN_SESSION_BUFFER_COUNT	= 2,
		EN_SESSION_MUTEX_COUNT		= 2,
	};

	enum enTimerID
	{
		SESSION_DISPATCH_TIMERID,
	};

	using namespace boost::asio::ip;

	class XAsioTCPSession;
	class XAsioTCPClient;
	class XAsioTCPServer;
	
	typedef boost::shared_ptr<class XAsioTCPSession>			TcpSessionPtr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::socket>		TcpSocketPtr;
	
	typedef boost::shared_ptr<class XAsioTCPClient>				TcpClientPtr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::resolver>	TcpResolverPtr;
			
	typedef boost::shared_ptr<class XAsioTCPServer>				TcpServerPtr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor>	TcpAcceptorPtr;
	
	/**
	 * TCP���ӻỰ
	 */
	class XGAME_API XAsioTCPSession : public XAsioSession, public XAsioTimer, public boost::enable_shared_from_this<XAsioTCPSession>
	{
	public:
		XAsioTCPSession( XAsioServiceController& controller );
		~XAsioTCPSession();

		/**
		 * ��ȡ�׽���
		 */
		const TcpSocketPtr	getSocket() const;
		
		/**
		 * �Ƿ��
		 */
		bool			isOpen();

		/**
		 * �رջỰ
		 */
		void			close();

		/**
		 * ��������
		 */
		virtual void	recv();

		/**
		 * ����ָ����������
		 */
		virtual void	recv( size_t bufferSize );

		/**
		 * ����
		 */
		virtual void	send( XAsioBuffer& buffer );

		/**
		 * ��ͣ����
		 * 
		 */
		void			suspendSend( bool b );
		/**
		 * ��ͣ�������ݵ��ɷ�
		 */
		void			suspendDispatch( bool b );
			
		/**
		 * ��ȡ�ѷ����ֽ�
		 */
		size_t			getSendSize() const;
		/**
		 * ��ȡ�ѽ����ֽ�
		 */
		size_t			getRecvSize() const;

	protected:
		/**
		 * �����Ͷ���
		 */
		bool			processSend();
		/**
		 * ������ն���
		 */
		bool			processRead();

		/**
		 * ֱ�ӷ��ͻ���
		 */
		void			sendImmediately( const XAsioBuffer& buffer );

		/**
		 * ��ʱ����Ӧ
		 */
		virtual bool	onTimer( unsigned int id, const void* pUserData );
		/**
		 * ��ɽ�����Ӧ����
		 */
		virtual void	onRecvCallback( const boost::system::error_code& err, size_t bytesTransferred );
		/**
		 * ��ɷ�����Ӧ����
		 */
		virtual void	onSendCallback( const boost::system::error_code& err, size_t bytesTransferred );
		/**
		 * �رյ���Ӧ����
		 */
		virtual void	onCloseCallback( const boost::system::error_code& err );

	protected:
		TcpSocketPtr			m_socket;
		
		typedef container::list<XAsioBuffer>	PACKET_CONAINER;
		/**
		 * ���շ��Ͷ���
		 */
		PACKET_CONAINER			m_packetBuffs[EN_SESSION_BUFFER_COUNT];		
		boost::mutex			m_mutexs[EN_SESSION_MUTEX_COUNT];

		/**
		 * �Ƿ����б�־
		 */
		bool					m_isSending;
		/**
		 * �Ƿ���ͣ����
		 */
		bool					m_isSuspendSend;
		/**
		 * �Ƿ���ͣ�ɷ�
		 */
		bool					m_isSuspendDispatch;

		/**
		 * �������ݳ���
		 */
		unsigned long			m_dwSendSize;
		/**
		 * �������ݳ���
		 */
		unsigned long			m_dwRecvSize;
	};

	/**
	 * TCP�ͻ���
	 */
	class XGAME_API XAsioTCPClient : public XAsioClientInterface, public boost::enable_shared_from_this<XAsioTCPClient>
	{
	public:
		XAsioTCPClient( XAsioServiceController& controller );
		~XAsioTCPClient();
		
		//virtual void	init();

		//virtual void	release();
		
		virtual void	connect( const std::string& host, uint16_t port );

		virtual void	connect( const std::string& host, const std::string& protocol );
		
		/**
		 * �������ӵ��������ɹ��Ļص�
		 */
		void			setConnectHandler( std::function<void( TcpSessionPtr )> );
		/**
		 * ���ô�������ʱ�Ļص�
		 */
		void			setReconnectHandler( std::function<void()> );
		
	protected:				
		/**
		 * �����̶߳���
		 * ���ؿ������ڴ�ع���
		 */
		virtual	TcpSessionPtr	createTCPSession();
		
		/**
		 * ���������ӵ���Ӧ
		 */
		virtual void	onResolveCallback( const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator it );
		
		/**
		 * ����ʱ����Ӧ
		 */
		virtual void	onConnectCallback( TcpSessionPtr session, const boost::system::error_code& err );
				
	protected:
		TcpResolverPtr							m_ptrResolver;
		TcpSessionPtr							m_ptrSession;

		std::function<void( TcpSessionPtr )>	m_funcConnectHandler;
		std::function<void()>					m_funcReconnectHandler;
	};

	/**
	 * TCP������
	 */
	class XGAME_API XAsioTCPServer : public XAsioServerInterface, public boost::enable_shared_from_this<XAsioTCPServer>
	{
	public:
		XAsioTCPServer( XAsioServiceController& controller );
		~XAsioTCPServer();
		
		const TcpAcceptorPtr	getAcceptor() const;
				
		/**
		 * ��ʼ����
		 * @param threadNum		�������߳�����
		 * @param port			�����Ķ˿�
		 */
		virtual void	startAccept( int threadNum, unsigned short port );

		/**
		 * ��ʼ����
		 * @param threadNum		�������߳�����
		 * @param port			�����Ķ˿�
		 */
		virtual void	startAccept( unsigned short port );

		/**
		 * ֹͣ����
		 */
		void			stopAccept();
		
	public:
		/**
		 * ��������Ӧ������
		 */
		void			setAcceptHandler( std::function<void( TcpSessionPtr )> handler );

		/**
		 * ֹͣ��������Ӧ������
		 */
		void			setCancelHandler( std::function<void()> handler );
		
	protected:
		
		void			processAccept();

		/**
		 * �����̶߳���
		 * ���ؿ������ڴ�ع���
		 */
		virtual	TcpSessionPtr	createTCPSession();

		/**
		 * ��Ӧ����������
		 */
		void			onAcceptCallback( TcpSessionPtr session, const boost::system::error_code& err );

	protected:
		TcpAcceptorPtr							m_acceptor;

		std::function<void( TcpSessionPtr )>	m_funcAcceptHandler;
		std::function<void()>					m_funcCancelHandler;
	};
}