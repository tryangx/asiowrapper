/**
 * ����ͻ��˷�װ
 */
#pragma once

#include "XAsioTCP.h"

namespace XASIO
{
	class XClient
	{
	public:
		/**
		 * ��̬��־���ƽӿ�
		 */
		static void		setLog( std::function<void( std::string& )> handler );
		static void		disableLog();

		/**
		 * �õ����пͻ��˷��͵ĳ���
		 */
		static size_t	getSendSize();
		/**
		 * �õ����пͻ��˽��յĳ���
		 */
		static size_t	getRecvSize();

	protected:
		static void		onLogHandler( std::string& err );
				
	protected:
		static std::function<void( std::string )>	m_sfuncLogHandler;
		/**
		 * ���пͻ��˷�����Ϣ�ĳ���
		 */
		static size_t		m_sizeSend;
		/**
		 * ���пͻ��˽�����Ϣ�ĳ���
		 */
		static size_t		m_sizeRecv;

	public:
		XClient( XAsioService& io );
		~XClient();

		/**
		 * ��ȡ����ӿ�
		 */
		TcpClientPtr	getService();
		
		/**
		 * �õ�ID
		 */
		unsigned int	getId() const;
		void			setId( unsigned int id );

		/**
		 * ���õ�ַ
		 */
		void		setAddress( std::string host = "localhost", int port = DEFAULT_XASIO_PORT );
		
		/**
		 * ����
		 */
		void		connect();
		/**
		 * �Ͽ�����
		 */
		void		disconnect();
				
		/**
		 * ����
		 */
		void		send();
		void		send( std::string content );
		void		send( XAsioBuffer& buff );

		/**
		 * ����
		 */
		void		testSend();		
		void		sendThread();

	protected:
		void		init();
		
		/**
		 * ��������
		 * ���ش˺����ɿ�����Ҫ��ȡ�ķ�ʽ����ͷ�ȵ�
		 */
		virtual void	recv();
		
		void		onConnect( TcpSessionPtr session );
		void		onRecv( XAsioBuffer& buff );
		void		onRecvComplete();
		void		onSend( size_t bytesTransferred );
		void		onResolve();
		void		onClose( size_t id );
		void		onLogInfo( const char* pInfo );
		void		onLog( std::string& err );

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
		unsigned int		m_id;

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
		 * �������ӳ�ʱ����ļ�ʱ��
		 */
		deadline_timer		m_connectTimer;

		/**
		 * ��ȡ��Ϣ���
		 */
		bool				m_bReadHeader;
		XAsioPackageHeader	m_packageHeader;

		/**
		 * ����
		 */
		boost::thread		m_sendThread;
	};
}