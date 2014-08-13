#include "../../include/db/XDBMysql.h"

namespace XMYSQL
{
#define CATCH_SQL_EXCEPTION		\
	catch( SQLException& e ) { onLog( e.what() ); }\
	catch( ... ) {}

	XDBMysql::XDBMysql() : m_pDriver( NULL ), m_pSavepoint( NULL ), m_pConnection( NULL ),
		m_iCurConnect( 0 ), m_iMaxConnect( 1 ), m_funcLogHandler( NULL )
	{
	}
	
	bool XDBMysql::isConnected() const { return m_pDriver != NULL; }

	bool XDBMysql::connect( const char* pAddress, const char* pUserName, const char* pPassword )
	{
		m_sAddress	= pAddress;
		m_sUserName	= pUserName;
		m_sPassword = pPassword;

		if ( m_pDriver )
		{
			close();
		}
		try
		{
			m_pDriver = mysql::get_mysql_driver_instance();
			return getConnection() != NULL;
		}
		CATCH_SQL_EXCEPTION
		return false;
	}

	void XDBMysql::selectSchema( const char* pSchemaName )
	{
		m_sSchema = pSchemaName;
		try
		{
			if ( m_pConnection )
			{
				m_pConnection->setSchema( m_sSchema.c_str() );
			}
		}
		CATCH_SQL_EXCEPTION
	}

	void XDBMysql::close()
	{
		if ( isConnected() )
		{
			mutex::scoped_lock lock( m_mutexConn );
			try
			{
				LIST_CONNECTION::iterator it;
				for ( it = m_listConnect.begin(); it != m_listConnect.end(); it++ )
				{
					Connection* pConn = *it;
					if ( !pConn->isClosed() )
					{
						pConn->close();
					}
				}
			}
			CATCH_SQL_EXCEPTION
		}
	}

	bool XDBMysql::execute( const char* pCmd, Connection* pConn /* = NULL */ )
	{
		if ( !pCmd || !isConnected() )
		{
			return false;
		}
		Statement* pState = createStatement( pConn );
		if ( !pState )
		{
			return false;
		}
		try
		{
			bool ret = pState->execute( pCmd );
			releaseConnection( pConn );
			return ret;
		}
		CATCH_SQL_EXCEPTION
		return false;
	}

	int XDBMysql::update( const char* pCmd, Connection* pConn /* = NULL */ )
	{
		if ( !pCmd || !isConnected() )
		{
			return 0;
		}
		Statement* pState = createStatement( pConn );
		if ( !pState )
		{
			return 0;
		}
		try
		{
			int count = pState->executeUpdate( pCmd );
			releaseConnection( pConn );
			return count;
		}
		CATCH_SQL_EXCEPTION
		return 0;
	}

	ResultSet* XDBMysql::query( const char* pCmd, Connection* pConn /* = NULL */ )
	{
		if ( !pCmd || !isConnected() )
		{
			return NULL;
		}
		Statement* pState = createStatement( pConn );
		if ( !pState )
		{
			return NULL;
		}
		try
		{
			ResultSet* pSet = pState->executeQuery( pCmd );
			releaseConnection( pConn );
			return pSet;
		}
		CATCH_SQL_EXCEPTION
		return NULL;
	}
	
	Savepoint* XDBMysql::crateSavePoint( Connection* pConn )
	{
		if ( !pConn )
		{
			return NULL;
		}
		try
		{
			return pConn->setSavepoint();
		}
		CATCH_SQL_EXCEPTION
		return NULL;
	}
	bool XDBMysql::rollback( Connection* pConn, Savepoint* pSavepoint /* = NULL */ )
	{
		if ( !pConn )
		{
			return false;
		}
		try
		{
			if ( pSavepoint )
			{
				pConn->rollback( pSavepoint );
			}
			else
			{
				pConn->rollback();
			}
		}
		CATCH_SQL_EXCEPTION
		return true;
	}
	void XDBMysql::releaseSavePoint( Connection* pConn, Savepoint* pSavepoint )
	{
		if ( !pConn || !pSavepoint )
		{
			return;
		}
		try
		{
			pConn->releaseSavepoint( pSavepoint );
		}
		CATCH_SQL_EXCEPTION
	}
	
	void XDBMysql::createPreparestatement( int type, const char* pCmd )
	{
		if ( !pCmd )
		{
			return;
		}
		Connection* pConn = getConnection();
		if ( !pConn )
		{
			return;
		}
		try
		{
			PreparedStatement* pStatment = pConn->prepareStatement( pCmd );
			if ( pStatment )
			{
				mutex::scoped_lock lock( m_mutexPrepareStatment );
				m_mapPrepareStatment.insert( std::make_pair( type, pStatment ) );
			}
		}
		CATCH_SQL_EXCEPTION
	}

	PreparedStatement* XDBMysql::getPreparestatement( int type )
	{
		mutex::scoped_lock lock( m_mutexPrepareStatment );
		MAP_PREPARESTATEMENT::iterator it = m_mapPrepareStatment.find( type );
		return it != m_mapPrepareStatment.end() ? it->second : NULL;
	}

	Statement* XDBMysql::createStatement( Connection* pConn /* = NULL */ )
	{
		if ( !pConn )
		{
			pConn = getPoolConnection();
		}		
		return pConn ? pConn->createStatement() : NULL;
	}

	Connection* XDBMysql::createConnection()
	{
		if ( !isConnected() )
		{
			return NULL;
		}
		Connection* pConn = NULL;
		try
		{
			pConn = m_pDriver->connect( m_sAddress, m_sUserName, m_sPassword );
			if ( pConn && !m_sSchema.empty() )
			{
				pConn->setSchema( m_sSchema );
			}
			return pConn;
		}
		CATCH_SQL_EXCEPTION
		return NULL;
	}
	Connection* XDBMysql::getConnection()
	{
		if ( !m_pConnection )
		{
			m_pConnection = createConnection();
		}
		return m_pConnection;
	}
	Connection* XDBMysql::getPoolConnection()
	{
		mutex::scoped_lock lock( m_mutexConn );
		Connection* pConn = NULL;	
		if ( m_listConnect.size() > 0 )
		{
			pConn = m_listConnect.front();
			m_listConnect.pop_front();
			if ( pConn->isClosed() )
			{
				delete pConn;
				pConn = createConnection();
			}
			if ( !pConn )
			{
				m_iCurConnect--;
			}
		}
		else if ( m_iCurConnect < m_iMaxConnect )
		{
			pConn = createConnection();
			if ( pConn )
			{
				m_iCurConnect++;
			}
		}
		return pConn;
	}
	void XDBMysql::initConnectionPool( size_t poolSize )
	{
		mutex::scoped_lock lock( m_mutexConn );
		Connection* pConn = NULL;
		for ( size_t i = 0; i < poolSize; i++ )
		{
			pConn = createConnection();
			if ( pConn ) 
			{
				m_listConnect.push_back( pConn );
				m_iCurConnect++;
			}
			else
			{
				onLog( "Create connection failed!" );
				break;
			}
		}
	}
	void XDBMysql::releaseConnection( Connection* pConnect )
	{
		if ( pConnect )
		{
			mutex::scoped_lock lock( m_mutexConn );
			m_listConnect.push_back( pConnect );
		}
	}
	void XDBMysql::destroyConnect( Connection* pConnect )
	{
		if ( pConnect )
		{
			try
			{
				pConnect->close();
			}
			CATCH_SQL_EXCEPTION
		}
	}
	void XDBMysql::destroyPool()
	{
		LIST_CONNECTION::iterator it;
		mutex::scoped_lock lock( m_mutexConn );
		for ( it = m_listConnect.begin(); it != m_listConnect.end(); it++ )
		{
			destroyConnect( *it );
		}
		m_iCurConnect = 0;
		m_listConnect.clear();
	}
	void XDBMysql::onLog( const char* pLog )
	{
		if ( m_funcLogHandler != NULL )
		{
			m_funcLogHandler( pLog );
		}
	}
}