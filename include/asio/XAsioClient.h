/**
 * ����ͻ��˷�װ
 */
#pragma once

#include "XAsioTCP.h"
#include "XAsioStat.h"
#include "../util/XSingleton.h"

namespace XGAME
{
	typedef XSingleton<XAsioStat>	XAsioStatClientAgent;

	class XGAME_API XClient
	{
	public:
		XClient( XAsioService& io );
		~XClient();

		/**
		 * ��ȡ����ӿ�
		 */
		TcpClientPtr	getServicePtr();
		XAsioTCPClient*	getService();
		
		/**
		 * �õ�ID
		 */
		unsigned int	getClientId() const;
		void			setClientId( unsigned int id );

		bool			isConnected() const;

		/**
		 * ���õ�ַ
		 */
		void			setAddress( std::string host = "localhost", int port = 6580 );
		const char*		getIp() const;
		unsigned short	getPort() const;
		
		/**
		 * ����
		 */
		bool			connect();
		/**
		 * �Ͽ�����
		 */
		void			disconnect();
				
		/**
		 * ����
		 */
		void			send( XAsioBuffer& buff );

		/**
		 * ����
		 */
		void			testEcho();

		/**
		 * �������ӵ�������ʱ�Ĵ���
		 * @param handler	���ӳɹ���ʧ��ʱ�Ļص���������Ϊ�ռ�ʧ�ܣ�ʧ��ԭ�����Ϊ��ʱ��
		 */
		void			setConnectHandler( std::function<void( XClient* )> handler );

		/**
		 * ���ùر�ʱ�Ĵ���
		 * ���ڷ����������Ͽ�����
		 */
		void			setCloseHandler( std::function<void( size_t )> handler );

		/**
		 * ���ý��յ�����ʱ�Ĵ���
		 */
		void			setRecvHandler( std::function<void( XClient*, XAsioRecvPacket& )> handler );

		/**
		 * ���ý��յ�����ʱ�Ĵ���
		 */
		void			setLogHandler( std::function<void( const char* )> handler );

	protected:
		/**
		 * �ͷŻỰ
		 */
		void			release();
		
		/**
		 * ��������
		 * ���ش˺����ɿ�����Ҫ��ȡ�ķ�ʽ����ͷ�ȵ�
		 */
		virtual void	recv();
		
		virtual void	onConnect( TcpSessionPtr session );
		virtual void	onRecv( XAsioBuffer& buff );
		void			onSend( size_t bytesTransferred );
		void			onResolve();
		void			onClose( size_t id );
		void			onLog( const char* pLog );
		
		void			onConnTimeoutCallback( const boost::system::error_code& err );
		void			onDisconnTimeCallback( const boost::system::error_code& err );

	protected:
		/**
		 * asio�������
		 */
		XAsioService&		m_service;
		/**
		 * ����ӿڣ��������ӷ�����
		 */
		TcpClientPtr		m_ptrTCPClient;
		/**
		 * �Ự�ӿڣ������շ���Ϣ
		 */
		TcpSessionPtr		m_ptrSession;

		/**
		 * ��ţ����ڳع���
		 */
		unsigned int		m_iClientId;

		/**
		 * ������Ϣ
		 */
		int					m_iPort;
		std::string			m_sHost;

		/**
		 * ����״̬
		 */
		bool				m_bInit;
		bool				m_bIsConnected;

		/**
		 * �������ӳ�ʱ�������жϴ���ļ�ʱ��
		 */
		deadline_timer		m_timer;

		/**
		 * ��ȡ��Ϣ���
		 */
		bool				m_bTestEcho;
		bool				m_bReadHeader;
		XAsioRecvPacket		m_recvPacket;
		
		std::function<void( size_t )>						m_funcCloseHandler;
		std::function<void( XClient*, XAsioRecvPacket& )>	m_funcRecvHandler;
		std::function<void( XClient* )>						m_funcConnectHandler;
		std::function<void( const char* )>					m_funcLogHandler;
	};
}