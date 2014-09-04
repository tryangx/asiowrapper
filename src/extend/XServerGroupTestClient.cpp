#include "extend/XServerGroup.h"
#include "util/XStringUtil.h"

namespace XGAME
{
	XTestClient::XTestClient( XAsioService& io ) : XClient( io )
	{
	}
	
	//-------test pool
	XTestClientPool::XTestClientPool() : m_allocateId( CLIENT_START_ID ), m_maxLimited(0)
	{
		m_ip = "localhost";
	}

	XTestClientPool::~XTestClientPool()
	{
		clear();
		m_mapClient.clear();
		m_mapTempClient.clear();
	}

	void XTestClientPool::clear()
	{
		closeClient( -1 );
		m_ioService.stopAllServices();
	}

	void XTestClientPool::setAddress( const char* ip, int port )
	{
		m_ip = ip;
		m_port = port;
	}

	void XTestClientPool::setMaxClient( unsigned int limit )
	{
		m_maxLimited = limit;
	}

	void XTestClientPool::send( XAsioSendPacket& packet )
	{
		XAsioBuffer buffer;
		packet.output( buffer );
		send( buffer );
	}

	void XTestClientPool::send( XAsioBuffer& buffer )
	{
		mutex::scoped_lock lock( m_mutexMap );

		MAP_CLINTPTR::iterator it = m_mapTempClient.begin();
		for ( ; it != m_mapTempClient.end(); it++ )
		{
			XTestClientPtr& ptr = it->second;
			ptr->send( buffer );
		}
	}

	unsigned int XTestClientPool::createClient()
	{
		mutex::scoped_lock lock( m_mutexMap );

		m_allocateId++;
		XTestClientPtr ptr = XTestClientPtr( new XTestClient( m_ioService ) );
		ptr->setClientId( m_allocateId );
		ptr->setAddress( m_ip, m_port );
		ptr->connect();
		ptr->setConnectHandler( std::bind( &XTestClientPool::onClientConnect, this, std::placeholders::_1 ) );
		ptr->setCloseHandler( std::bind( &XTestClientPool::onClientClose, this, std::placeholders::_1 ) );
		m_ioService.startService( ptr->getService(), 1 );
		m_mapClient.insert( std::make_pair( ptr->getClientId(), ptr ) );
		return ptr->getClientId();
	}

	void XTestClientPool::closeClient( int id )
	{
		mutex::scoped_lock lock( m_mutexMap );

		if ( !m_mapTempClient.empty() )
		{
			MAP_CLINTPTR::iterator it = m_mapTempClient.begin();
			for ( ; it != m_mapTempClient.end(); )
			{
				XTestClientPtr& ptr = it->second;
				if ( !ptr->isConnected() )
				{
					it = m_mapTempClient.erase( it );
				}
				else
				{
					it++;
				}
			}
		}
		if ( !m_mapClient.empty() )
		{
			if ( id > 0 )
			{
				onClientClose( id );
			}
			else
			{
				int inx = 0;
				int needClose = id == -1 ? m_mapClient.size() : rand() % 3 + 1;
				for ( int left = 0; left < needClose && !m_mapClient.empty(); left++ )
				{
					int targetIndex = rand() % m_mapClient.size();					
					MAP_CLINTPTR::iterator it = m_mapClient.begin();
					for ( ; it != m_mapClient.end(); it++, inx++ )
					{
						if ( inx != targetIndex )
						{
							continue;
						}
						XTestClientPtr& ptr = it->second;
						onClientClose( ptr->getClientId() );
						break;
					}
				}			
			}
		}
	}

	void XTestClientPool::forEachClient( std::function<void(XTestClient*)> handler )
	{
		mutex::scoped_lock lock( m_mutexMap );
		MAP_CLINTPTR::iterator it = m_mapClient.begin();
		for ( ; it != m_mapClient.end(); it++ )
		{
			XTestClientPtr& ptr = it->second;
			handler( ptr.get() );
		}
	}

	void XTestClientPool::onLog( const char* pLog )
	{
		if ( m_funcLogHandler != nullptr )
		{
			std::string s = "[client]";
			s += pLog;
			m_funcLogHandler( s.c_str() );
		}
	}

	void XTestClientPool::onClientConnect( XClient* p )
	{
		onLog( outputString( "%d connect %s:%d", p->getClientId(), p->getIp(), p->getPort() ) );
	}

	void XTestClientPool::onClientClose( unsigned int id )
	{
		MAP_CLINTPTR::iterator it = m_mapClient.find( id );
		if ( it != m_mapClient.end() )
		{
			XTestClientPtr& ptr = it->second;
			if ( ptr->isConnected() )
			{
				ptr->disconnect();
				m_ioService.removeService( ptr->getService() );
				m_mapTempClient.insert( std::make_pair( ptr->getClientId(), ptr ) );
				m_mapClient.erase( it );
			}
		}
	}
}