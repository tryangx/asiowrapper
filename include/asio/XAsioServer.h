#pragma once

#include "XAsioTCP.h"

#include <unordered_map>

namespace XASIO
{
	class XServer;
	class XServerSession;

	typedef boost::shared_ptr<class XServerSession>		ServerSessionPtr;

	//---------------------------
	//	���ӵ��������ĻỰ
	class XServerSession : public boost::enable_shared_from_this<XServerSession>
	{
	public:
		static ServerSessionPtr	create( TcpSessionPtr ptr );
		
		static void		setLog( std::function<void( std::string& )>& handler );
		static void		disableLog();
		/**
		 * �õ����յ��ֽ���
		 */
		static size_t	getRecvSize();

	public:
		XServerSession();
		XServerSession( TcpSessionPtr ptr );

		/**
		 * ��ʼ��
		 * @param ptr ָ���Ự�Ķ���
		 */
		void		init( TcpSessionPtr ptr = nullptr );

		/**
		 * �Ƿ�������
		 */
		bool		isStarted();

		/**
		 * �Ƿ���ֹͣ
		 */
		bool		isStoped();

		/**
		 * ����
		 */
		void		send( XAsioBuffer& buff );

		/**
		 * �Ͽ�
		 */
		void		close();

		/**
		 * ���Է���
		 */
		void		testSend();
		void		sendThread();

	protected:
		/**
		 * ��������
		 */
		void			recv();

		void			onLog( std::string& err );
		void			onLogInfo( const char* pInfo );

		virtual void	onClose();
		virtual void	onRecv( XAsioBuffer buff );
		virtual void	onRecvComplete();
		virtual void	onWrite( size_t bytesTransferred );

	protected:
		static void		onLogHandler( std::string& err );

	protected:
		static std::function<void( std::string )>	m_sfuncLogHandler;
		static size_t		m_sizeRecv;

		TcpSessionPtr		m_tcpSession;

		bool				m_bIsStarted;
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;
		
		boost::thread		m_sendThread;

		friend				XServer;
	};

	class XServer : public XAsioPool<XServerSession>
	{
	public:
		XServer( XAsioService& service );
		~XServer();

		/**
		 * ��ȡ����
		 */
		TcpServerPtr		getService();

		/**
		 * ��ȡ�Ự
		 */
		ServerSessionPtr	getSession( unsigned int id );

		/**
		 * �������ӵ�ַ
		 */
		void		setAddress( int port );

		/**
		 * �����������߳�����
		 */
		void		setAcceptThreadNum( int threadNum );

		/**
		 * ����������
		 */
		void		startServer();
		/**
		 * ֹͣ������
		 */
		void		stopServer();

		/**
		 * ���͸�ָ��ID���
		 */
		void		sendTo( int id, XAsioBuffer& buff );
		/**
		 * ���͸�ȫ��
		 */
		void		sendToAll( XAsioBuffer& buff );		
		/**
		 * ���͸�ָ������
		 */
		void		sendToIdList( XAsioBuffer& buff, std::vector<unsigned int>& v );

		void		testSend();

	public:
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		virtual void init();
		virtual void release();
		/**
		 * ��ȡ��Ч��ID
		 */
		unsigned int	queryValidId();

		void		onStartServer();
		void		onAccept( TcpSessionPtr );
		void		onCancel();
		void		onSessionClose( size_t id );
		void		onLog( std::string& err );
		void		onLogInfo( const char* pInfo );

	protected:
		XAsioService&					m_service;

		TcpServerPtr					m_ptrTCPServer;

		typedef std::tr1::unordered_map<size_t, ServerSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;

		boost::mutex					m_mutex;
		//config
		int								m_iAcceptThreadNum;
		//address info
		int								m_iPort;
		//status
		bool							m_bIsStarted;

		std::function<void( std::string& )>			m_funcLogHandler;
	};
}