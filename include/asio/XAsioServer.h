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
	class XServerSession;

	typedef boost::shared_ptr<class XServerSession>		XServerSessionPtr;
	
	typedef XSingleton<XAsioStat>	XAsioStatServerAgent;

	//---------------------------
	//	���ӵ��������ĻỰ
	class XGAME_API XServerSession : public boost::enable_shared_from_this<XServerSession>
	{
	public:
		static XServerSessionPtr	create( TcpSessionPtr ptr );

	protected:
		static std::function<void( const char* )>			m_sfuncLogHandler;

	public:
		XServerSession();
		XServerSession( TcpSessionPtr ptr );
		~XServerSession();

		/**
		 * ��ʼ��
		 * @param ptr ָ���Ự�Ķ���
		 */
		void			init( TcpSessionPtr ptr = nullptr );

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
		 * ��־�Ĵ���
		 */
		void			setLogHandler( std::function<void( const char* )> handler );
		/**
		 * ���յĴ���
		 */
		void			setRecvHandler( std::function<void( XServerSession*, XAsioRecvPacket& )> handler );
		
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

		std::function<void( XServerSession*, XAsioRecvPacket& )>	m_funcRecvHandler;
		std::function<void( const char* )>		m_funcLogHandler;
	};

	//  ������
	class XServer // : public XAsioPool<XServerSession>
	{
	public:
		XServer( XAsioService& service );
		~XServer();

		/**
		 * ��ȡ����
		 */
		TcpServerPtr		getService();

		/**
		 * �Ƿ�����
		 */
		bool				isStarted();

		/**
		 * ��ȡ�Ự
		 */
		XServerSession*		getSession( unsigned int id );
		
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

		/**
		 * ��ȡ�ͻ�������
		 */
		size_t		getClientCount();

		//--------������Ϣ����------------

		/**
		 * ��ȡδ�������Ϣ
		 */
		bool		queryRecvPacket( XAsioRecvPacket& packet );
		
	public:
		void		setLogHandler( std::function<void( const char* )> handler );
		void		setConnectHandler( std::function<void( XServerSession* )> handler );
		
	protected:
		/**
		 * �����Ự
		 */
		virtual XServerSession*	createSession();

		unsigned int	queryValidId();
		
		void			onStartServer();
		void			onAccept( TcpSessionPtr );
		void			onCancel();
		void			onSessionClose( size_t id );		
		void			onSessionRecv( XServerSession* pSession, XAsioRecvPacket& packet );
		void			onLog( const char* pLog );

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
		typedef std::tr1::unordered_map<size_t, XServerSessionPtr>	MAPSERVERSESSIONPTR;
		MAPSERVERSESSIONPTR				m_mapSession;
		unsigned int					m_iAllocateId;
		boost::mutex					m_sessionMutex;

		//�����߳�����
		int								m_iAcceptThreadNum;

		//ʹ�ö˿�
		int								m_iPort;

		//�Ƿ�����
		bool							m_bIsStarted;

		//������Ϣ����
		std::list<XAsioRecvPacket>		m_listRecvPackets;
		boost::mutex					m_packetMutex;

		std::function<void( const char* )>			m_funcLogHandler;
		std::function<void( XServerSession* )>		m_funcAcceptHandler;
	};
}