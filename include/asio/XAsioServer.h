/**
 * �����������װ
 */
#pragma once

#include "XAsioTCP.h"
#include "XAsioStat.h"
#include "../util/XSingleton.h"

#include <unordered_map>

namespace XGAME
{
	//ǰ10000�ű���
#define CLIENT_START_ID		10000

	class XServer;
	class XAsioTCPSrvSession;

	typedef boost::shared_ptr<class XAsioTCPSrvSession>		TCPSrvSessionPtr;
	
	//---------------------------
	//	���ӵ��������ĻỰ
	class XGAME_API XAsioTCPSrvSession : public boost::enable_shared_from_this<XAsioTCPSrvSession>
	{
	public:
		static TCPSrvSessionPtr	create( TcpSessionPtr ptr );

	protected:
		static std::function<void( const char* )>			m_sfuncLogHandler;

	public:
		XAsioTCPSrvSession();
		XAsioTCPSrvSession( TcpSessionPtr ptr );
		~XAsioTCPSrvSession();

		/**
		 * ��ʼ��
		 * @param ptr ָ���Ự�Ķ���
		 */
		void			init( TcpSessionPtr ptr = nullptr );

		void			setStat( XAsioStat* pStat );

		/**
		 * �õ��ỰID
		 */
		unsigned int	getSessionId();

		/**
		 * �ͷŻỰ���handler
		 */
		void			release();

		/**
		 * �Ƿ�������
		 */
		bool			isStarted();

		/**
		 * �Ƿ���ֹͣ
		 */
		bool			isOpen();

		/**
		 * ����
		 */
		void			send( XAsioBuffer& buff );

		/**
		 * �Ͽ�
		 */
		void			close();
		
		/**
		 * �Ƿ�ʱ
		 */
		bool			isTimeout();

		//---------�ص��������
		/**
		 * ��־�Ĵ���
		 */
		void			setLogHandler( std::function<void( const char* )> handler );
		/**
		 * ���յĴ���
		 */
		void			setRecvHandler( std::function<void( XAsioTCPSrvSession*, XAsioRecvPacket& )> handler );
		
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
		TcpSessionPtr		m_ptrSession;

		/**
		* ��ȡ��Ϣ���
		*/
		bool				m_bIsStarted;
		XAsioRecvPacket		m_recvPacket;		
		long long			m_lastTickerTime;

		std::function<void( XAsioTCPSrvSession*, XAsioRecvPacket& )>	m_funcRecvHandler;
		std::function<void( const char* )>		m_funcLogHandler;

		XAsioStat*			m_pStat;
	};

	//---------------------
	//  ������
	class XServer // : public XAsioPool<XAsioTCPSrvSession>
	{
	public:
		XServer( XAsioService& service );
		~XServer();

		/**
		 * ��ȡ����
		 */
		TcpServerPtr		getService();

		XAsioStat*			getStat();

		/**
		 * �Ƿ�����
		 */
		bool				isStarted();

		/**
		 * ��ȡ�Ự
		 */
		XAsioTCPSrvSession*		getSession( unsigned int id );
		
		/**
		 * �رջỰ
		 */
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

		//--------�ͻ��˴���

		/**
		 * ��ȡ�ͻ�������
		 */
		size_t		getClientCount();

		/**
		 * ˢ�¿ͻ���
		 * ���ڴ���ʱ�����
		 */
		void		updateClient();

		//--------������Ϣ����------------

		/**
		 * ��ȡδ�������Ϣ
		 */
		bool		queryRecvPacket( XAsioRecvPacket& packet );
		
	public:
		void		setLogHandler( std::function<void( const char* )> handler );
		void		setConnectHandler( std::function<void( XAsioTCPSrvSession* )> handler );
		
	protected:
		/**
		 * �����Ự
		 */
		virtual XAsioTCPSrvSession*	createSession();

		unsigned int	queryValidId();
		
		void			onStartServer();
		void			onAccept( TcpSessionPtr );
		void			onCancel();
		void			onSessionClose( size_t id );		
		void			onSessionRecv( XAsioTCPSrvSession* pSession, XAsioRecvPacket& packet );
		void			onLog( const char* pLog );
		void			onTimerUpdateCallback( const boost::system::error_code& err );

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
		typedef std::tr1::unordered_map<size_t, TCPSrvSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;
		boost::mutex					m_sessionMutex;

		//��ʱ��ʱ��
		deadline_timer					m_timerUpdateTimer;

		//�����߳�����
		int								m_iAcceptThreadNum;

		//ʹ�ö˿�
		int								m_iPort;

		//�Ƿ�����
		bool							m_bIsStarted;

		//������Ϣ����
		std::list<XAsioRecvPacket>		m_listRecvPackets;
		boost::mutex					m_packetMutex;

		XAsioStat						m_stat;

		std::function<void( const char* )>			m_funcLogHandler;
		std::function<void( XAsioTCPSrvSession* )>		m_funcAcceptHandler;
	};
}