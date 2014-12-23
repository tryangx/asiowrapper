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
		XClient( XAsioServiceController& controller );
		~XClient();

		/**
		 * ��ȡ����ӿ�
		 */
		XAsioTCPClient*	getService();
		
		/**
		 * �õ�ID
		 */
		unsigned long	getClientId() const;
		/**
		 * ����ID
		 */
		void			setClientId( unsigned long id );
		
		/**
		 * �Ƿ�������
		 */
		bool			isConnected() const;

		/**
		 * �����Ƿ�ʱ�Ͽ�
		 */
		void			setTimeoutDisconnect( bool enable );

		/**
		 * ���õ�ַ
		 */
		void			setAddress( std::string host = "localhost", int port = 6580 );
		/**
		 * ��ȡIP
		 */
		const char*		getIp() const;
		/**
		 * ��ȡ�˿�
		 */
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
		 * �������ӵ�������ʱ�Ĵ���
		 * @param handler	���ӳɹ���ʧ��ʱ�Ļص���������Ϊ�ռ�ʧ�ܣ�ʧ��ԭ�����Ϊ��ʱ��
		 */
		void			setConnectHandler( std::function<void( XClient* )> handler );

		/**
		 * ���ùر�ʱ�Ĵ���
		 * ���ڷ����������Ͽ�����
		 */
		void			setCloseHandler( std::function<void( unsigned long )> handler );

		/**
		 * ���ý��յ�����ʱ�Ĵ���
		 */
		void			setRecvHandler( std::function<void( XClient*, XAsioRecvPacket& )> handler );
		
		/**
		 * ������Ϣ�շ�
		 */
		void			testEcho();

	protected:
		/**
		 * ��������
		 * ���ش˺����ɿ�����Ҫ��ȡ�ķ�ʽ����ͷ�ȵ�
		 */
		virtual void	recv();
		
		virtual void	onConnect( TcpSessionPtr session );

		virtual void	onRecv( XAsioBuffer& buff );

		void			onSend( size_t bytesTransferred );

		void			onClose( size_t id );

		void			onLog( const char* pLog );
		
		void			onConnTimeoutCallback( const boost::system::error_code& err );

		void			onDisconnTimeCallback( const boost::system::error_code& err );

	protected:
		/**
		 * asio�������
		 */
		XAsioServiceController&		m_controller;
		/**
		 * ����ӿڣ��������ӷ�����
		 */
		TcpClientPtr		m_ptrTCPClient;
		/**
		 * �Ự�ӿڣ�������ӿ��л�ȡ
		 */
		TcpSessionPtr		m_ptrSession;

		/**
		 * ��ţ����ڳع���
		 */
		unsigned long		m_dwClientId;
				
		/**
		 * ����IP��ַ
		 */
		std::string			m_sHost;
		/**
		 * ���Ӷ˿�
		 */
		int					m_iPort;

		/**
		 * �Ƿ����ӵ�������
		 */
		bool				m_bIsConnected;

		/**
		 * �Ƿ�����ʱ�Ͽ�����
		 */
		bool				m_bTimeoutDisconnect;
				
		/**
		 * ��ȡ��Ϣ���
		 */		
		bool				m_bReadHeader;
		XAsioRecvPacket		m_recvPacket;

		/**
		 * �������ӳ�ʱ�������жϴ���ļ�ʱ��
		 */
		deadline_timer		m_timer;
		
		std::function<void( XClient* )>						m_funcConnectHandler;
		std::function<void( unsigned long )>				m_funcCloseHandler;
		std::function<void( XClient*, XAsioRecvPacket& )>	m_funcRecvHandler;

		/**
		 * ������Ϣ�շ�
		 */
		bool				m_bTestEcho;
	};
}