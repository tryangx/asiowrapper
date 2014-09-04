#include "db/XDBMysql.h"

namespace XGAME
{
#define CATCH_SQL_EXCEPTION		\
	catch( SQLException& e ) { onLog( e.what() ); }\
	catch( ... ) {}

	XDBMysql::XDBMysql() : m_pDriver( NULL ), m_pSavepoint( NULL ), m_pConnection( NULL ),
		m_iCurConnect( 0 ), m_iMaxConnect( 1 ), m_funcLogHandler( NULL )
	{
	}

	XDBMysql::~XDBMysql()
	{
		MAP_PREPARESTATEMENT::iterator it;
		for ( it = m_mapPrepareStatment.begin(); it != m_mapPrepareStatment.end(); it++ )
		{
			PreparedStatement* p = it->second;
			delete p;
		}
		m_mapPrepareStatment.clear();

		close();
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

	void XDBMysql::setAutoCommit( Connection* pConn, bool isAutoCommit )
	{
		try
		{
			if ( pConn )
			{
				pConn->setAutoCommit( isAutoCommit );
			}
		}
		CATCH_SQL_EXCEPTION
	}

	void XDBMysql::close()
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
				delete pConn;
			}
			m_listConnect.clear();

			if ( m_pConnection )
			{
				if ( !m_pConnection->isClosed() )
				{
					m_pConnection->close();
				}
				delete m_pConnection;
				m_pConnection = NULL;
			}
		}
		CATCH_SQL_EXCEPTION
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
			delete pState;
			releaseConnection( pConn );
			return ret;
		}
		CATCH_SQL_EXCEPTION
		releaseConnection( pConn );
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
			delete pState;
			releaseConnection( pConn );
			return count;
		}
		CATCH_SQL_EXCEPTION
		releaseConnection( pConn );
		return 0;
	}

	RESULTSET_PTR XDBMysql::query( const char* pCmd, Connection* pConn /* = NULL */ )
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
			delete pState;
			releaseConnection( pConn );
			return RESULTSET_PTR( pSet );		//need delete manual
		}
		CATCH_SQL_EXCEPTION
		releaseConnection( pConn );
		return NULL;
	}

	void XDBMysql::commit( Connection* pConn )
	{
		try
		{
			if ( pConn )
			{
				pConn->commit();
			}
		}
		CATCH_SQL_EXCEPTION
	}
	
	Savepoint* XDBMysql::createSavePoint( std::string& sName )
	{
		try
		{
			Connection* pConn = getConnection();
			return pConn ? pConn->setSavepoint( sql::SQLString( sName ) ) : NULL;
		}
		CATCH_SQL_EXCEPTION
		return NULL;
	}
	bool XDBMysql::rollback( Savepoint* pSavepoint /* = NULL */ )
	{
		try
		{
			Connection* pConn = getConnection();
			if ( pConn )
			{
				if ( pSavepoint )
				{
					pConn->rollback( pSavepoint );
				}
				else
				{
					pConn->rollback();
				}
				return true;
			}
		}
		CATCH_SQL_EXCEPTION
		return false;
	}
	void XDBMysql::releaseSavePoint( Savepoint* pSavepoint )
	{
		if ( !pSavepoint )
		{
			return;
		}
		try
		{
			Connection* pConn = getConnection();
			return pConn ? pConn->releaseSavepoint( pSavepoint ) : NULL;
		}
		CATCH_SQL_EXCEPTION
	}
	
	void XDBMysql::createPreparestatement( int type, const char* pCmd, Connection* pConn )
	{
		if ( !pCmd )
		{
			return;
		}
		if ( !pConn )
		{
			pConn = getPoolConnection();
		}
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

	Statement* XDBMysql::createStatement( Connection*& pConn )
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
		if ( pConnect != m_pConnection )
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
	void XDBMysql::setLogHandler( std::function<void( const char* )> handler )
	{
		m_funcLogHandler = handler;
	}
}