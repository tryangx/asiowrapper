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
		 * 连接数据库
		 */
		bool		connect( const char* pAddress, const char* pUserName, const char* pPassword );

		/**
		 * 选择架构
		 */
		void		selectSchema( const char* pSchemaName );

		/**
		 * 关闭连接
		 */
		void		close();

		/**
		 * 获取连接
		 */
		Connection*	getConnection();

		/**
		 * 执行语句
		 * @param pCmd	执行的语句，insert, update, delete
		 * @param pConn	默认使用的连接
		 */
		bool		execute( const char* pCmd, Connection* pConn = NULL );

		/**
		 * 更新
		 * create table, drop table, insert, update, delete
		 */
		int			update( const char* pCmd, Connection* pConn = NULL );

		/**
		 * 查询
		 * 主要用于select
		 */
		ResultSet*	query( const char* pCmd, Connection* pConn = NULL );
				
		/**
		 * 创建保存点
		 * innoDB
		 */
		Savepoint*	crateSavePoint( Connection* pConn );
		/**
		 * 释放保存点
		 */
		void		releaseSavePoint( Connection* pConn, Savepoint* pSavepoint );
		/**
		 * 回滚
		 */
		bool		rollback( Connection* pConn, Savepoint* pSavepoint = NULL );		

		/**
		 * 创建预处理
		 */
		void		createPreparestatement( int type, const char* pCmd );
		/**
		 * 获取预处理
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
		//组件
		Driver*					m_pDriver;
		Connection*				m_pConnection;
		Savepoint*				m_pSavepoint;

		//数据信息
		std::string				m_sUserName;
		std::string				m_sPassword;
		std::string				m_sAddress;
		std::string				m_sSchema;

		//连接
		typedef std::list<Connection*>		LIST_CONNECTION;
		LIST_CONNECTION			m_listConnect;
		int						m_iCurConnect;
		int						m_iMaxConnect;
		boost::mutex			m_mutexConn;		

		//预处理语句
		typedef boost::unordered_map<int,PreparedStatement*>	MAP_PREPARESTATEMENT;
		MAP_PREPARESTATEMENT	m_mapPrepareStatment;
		boost::mutex			m_mutexPrepareStatment;
		
		//std::function<void( const char* )>	m_funcLogHandler;

		boost::function<void( const char* )>	m_funcLogHandler;
	};
}