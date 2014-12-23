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
	/**
	 * ���ӵ��������ĻỰ
	 */
	class XGAME_API XAsioTCPSrvSession : public boost::enable_shared_from_this<XAsioTCPSrvSession>
	{
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

		XAsioStat*			m_pStat;
	};

	typedef boost::shared_ptr<class XAsioTCPSrvSession>		TCPSrvSessionPtr;
	//---------------------
	//  ������
	class XServer // : public XAsioPool<XAsioTCPSrvSession>
	{
	public:
		XServer( XAsioServiceController& controller );
		~XServer();

		/**
		 * ��ȡ����
		 */
		TcpServerPtr		getService();
		
		/**
		 * ��ȡIO״̬ͳ����
		 */
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
		void					closeSession( unsigned int id );

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
		XAsioServiceController&			m_controller;

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

		std::function<void( XAsioTCPSrvSession* )>		m_funcAcceptHandler;
	};
}