#pragma once

#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>
#include <cppconn/exception.h>
#include <list>
#include <unordered_map>
#include <boost/thread/mutex.hpp>

namespace XMYSQL
{
	using namespace sql;
	using namespace boost;

#define MAX_DBSQL_LENGTH		4096

	class XDBMysql
	{
	public:
		bool		isConnected() const;

		/**
		 * �������ݿ�
		 */
		bool		connect( const char* pAddress, const char* pUserName, const char* pPassword );

		/**
		 * �ر�����
		 */
		void		close();

		bool		selectDataBase( const char* pDataBase );

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
		 */
		Savepoint*	crateSavePoint( Connection* pConn );
		bool		rollback( Connection* pConn, Savepoint* pSavepoint = NULL );
		void		releaseSavePoint( Connection* pConn, Savepoint* pSavepoint );

		/**
		 * ����Ԥ����
		 */
		void		createPreparestatement( int type, const char* pCmd );
		/**
		 * ��ȡԤ����
		 */
		PreparedStatement*	getPreparestatement( int type );

	protected:
		XDBMysql();

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

		//����
		typedef std::list<Connection*>		LIST_CONNECTION;
		LIST_CONNECTION			m_listConnect;
		int						m_iCurConnect;
		int						m_iMaxConnect;
		boost::mutex			m_mutexConn;		

		//Ԥ�������
		typedef std::unordered_map<int,PreparedStatement*>	MAP_PREPARESTATEMENT;
		MAP_PREPARESTATEMENT	m_mapPrepareStatment;
		boost::mutex			m_mutexPrepareStatment;
	};
}