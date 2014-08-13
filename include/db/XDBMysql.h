#pragma once

#pragma warning( disable : 4251 )
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>
#include <cppconn/exception.h>
#include <list>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

namespace XMYSQL
{
	using namespace sql;
	using namespace boost;

#define MAX_DBSQL_LENGTH		4096

	class XDBMysql
	{
	public:
		XDBMysql();

		bool		isConnected() const;

		/**
		 * �������ݿ�
		 */
		bool		connect( const char* pAddress, const char* pUserName, const char* pPassword );

		/**
		 * ѡ��ܹ�
		 */
		void		selectSchema( const char* pSchemaName );

		/**
		 * �ر�����
		 */
		void		close();

		/**
		 * ��ȡ����
		 */
		Connection*	getConnection();

		/**
		 * ִ�����
		 * @param pCmd	ִ�е���䣬insert, update, delete
		 * @param pConn	Ĭ��ʹ�õ�����
		 */
		bool		execute( const char* pCmd, Connection* pConn = NULL );

		/**
		 * ����
		 * create table, drop table, insert, update, delete
		 */
		int			update( const char* pCmd, Connection* pConn = NULL );

		/**
		 * ��ѯ
		 * ��Ҫ����select
		 */
		ResultSet*	query( const char* pCmd, Connection* pConn = NULL );
				
		/**
		 * ���������
		 * innoDB
		 */
		Savepoint*	crateSavePoint( Connection* pConn );
		/**
		 * �ͷű����
		 */
		void		releaseSavePoint( Connection* pConn, Savepoint* pSavepoint );
		/**
		 * �ع�
		 */
		bool		rollback( Connection* pConn, Savepoint* pSavepoint = NULL );		

		/**
		 * ����Ԥ����
		 */
		void		createPreparestatement( int type, const char* pCmd );
		/**
		 * ��ȡԤ����
		 */
		PreparedStatement*	getPreparestatement( int type );

		template< typename HANDLER, typename OBJECT >
		void		setLogHandler( HANDLER eventHandler, OBJECT* eventHandlerObject ) { m_funcLogHandler = std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ); }

	protected:
		Statement*	createStatement( Connection* pConn = NULL );

		Connection*	getPoolConnection();
		Connection*	createConnection();

		void		initConnectionPool( size_t poolSize );
		void		releaseConnection( Connection* pConnect );
		void		destroyConnect( Connection* pConnect );
		void		destroyPool();

		void		onLog( const char* pLog );

	protected:
		//���
		Driver*					m_pDriver;
		Connection*				m_pConnection;
		Savepoint*				m_pSavepoint;

		//������Ϣ
		std::string				m_sUserName;
		std::string				m_sPassword;
		std::string				m_sAddress;
		std::string				m_sSchema;

		//����
		typedef std::list<Connection*>		LIST_CONNECTION;
		LIST_CONNECTION			m_listConnect;
		int						m_iCurConnect;
		int						m_iMaxConnect;
		boost::mutex			m_mutexConn;		

		//Ԥ�������
		typedef boost::unordered_map<int,PreparedStatement*>	MAP_PREPARESTATEMENT;
		MAP_PREPARESTATEMENT	m_mapPrepareStatment;
		boost::mutex			m_mutexPrepareStatment;
		
		//std::function<void( const char* )>	m_funcLogHandler;

		boost::function<void( const char* )>	m_funcLogHandler;
	};
}