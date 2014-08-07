/**
 * �����������װ
 */
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
		
		/**
		 * ��̬��־���ƽӿ�
		 */
		static void		setLog( std::function<void( const char* )> handler );
		static void		disableLog();

		/**
		 * �õ����лỰ���յ���Ϣ����
		 */
		static size_t	getRecvSize();
		static size_t	getSendSize();

	protected:
		static void		onLogHandler( const char* pLog );

	protected:
		static std::function<void( const char* )>	m_sfuncLogHandler;
		static size_t		m_staSizeRecv;
		static size_t		m_staSizeSend;

	public:
		XServerSession();
		XServerSession( TcpSessionPtr ptr );

		/**
		 * ��ʼ��
		 * @param ptr ָ���Ự�Ķ���
		 */
		void		init( TcpSessionPtr ptr = nullptr );

		void		release();

		/**
		 * �Ƿ�������
		 */
		bool		isStarted();

		/**
		 * �Ƿ���ֹͣ
		 */
		bool		isOpen();

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
		void		sendTestPackage();

	protected:
		void			recv();

		void			onLog( const char* pLog );

		virtual void	onClose();
		virtual void	onRecv( XAsioBuffer& buff );
		virtual void	onWrite( size_t bytesTransferred );

	protected:
		/**
		 * �Ự�ӿڣ����ڷ�����Ϣ
		 */
		TcpSessionPtr		m_tcpSession;

		/**
		* ��ȡ��Ϣ���
		*/
		bool				m_bIsStarted;
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;
		XAsioPackage		m_lastPackage;
		
		/**
		 * ���Ͳ�����
		 */
		boost::shared_ptr<boost::thread>		m_sendThread;

		//friend				XServer;
	};

	//  ������
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
		
		void				closeSession( unsigned int id );

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

		/**
		 * ��ȡ�ͻ�������
		 */
		size_t		getClientCount();

		/**
		 * ���Է���
		 */
		void		testSend();
		void		testSendDirectly();

	public:
		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }
		
	protected:
		unsigned int	queryValidId();

		ServerSessionPtr	createSession();

		void		onStartServer();
		void		onAccept( TcpSessionPtr );
		void		onCancel();
		void		onSessionClose( size_t id );
		void		onLog( const char* pLog );

	protected:
		/**
		 * asio�������
		 */
		XAsioService&					m_service;

		/**
		 * ����ӿڣ����ڷ���������
		 */
		TcpServerPtr					m_ptrTCPServer;

		/**
		 * �������������Ự����
		 */
		typedef std::tr1::unordered_map<size_t, ServerSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;

		boost::mutex					m_mutex;

		//�����߳�����
		int								m_iAcceptThreadNum;

		//ʹ�ö˿�
		int								m_iPort;

		//�Ƿ�����
		bool							m_bIsStarted;

		std::function<void( const char* )>			m_funcLogHandler;
	};
}