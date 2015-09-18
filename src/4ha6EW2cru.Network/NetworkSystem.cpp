#include "NetworkSystem.h"

#include "NetworkSystemScene.h"

#include <RakNetworkFactory.h>
#include <RakSleep.h>
#include <BitStream.h>
#include <MessageIdentifiers.h>

using namespace RakNet;
using namespace std;

#include "Logging/Logger.h"
using namespace Logging;

#include "Events/Event.h"
#include "Events/EventData.hpp"
using namespace Events;

#include "System/SystemType.hpp"
using namespace Configuration;

#include "Management/Management.h"

#include "NetworkUtils.h"

#include "Configuration/ConfigurationTypes.hpp"

#include "ServerNetworkProvider.h"
#include "ClientNetworkProvider.h"

namespace Network
{
	NetworkSystem::~NetworkSystem( )
	{
		delete m_networkProvider;
		m_networkProvider = 0;
	}

	void NetworkSystem::Release( )
	{
		try{
		m_networkProvider->Release();
		}catch(...){
			cerr << "API Failed, to call the API m_networkProvider->Release()\n";
			throw;
		}
	}

	ISystemScene* NetworkSystem::CreateScene()
	{
		try{
		m_scene = new NetworkSystemScene( this );
		}catch(...){
	        cerr << "Unable to allocate the Requested Memory\n";
	        delete m_scene;
	        cerr << abort();
		}
		return m_scene;
	}

	void NetworkSystem::Initialize( Configuration::IConfiguration* configuration )
	{
		m_configuration = configuration;

		if ( m_attributes[ System::Attributes::Network::IsServer ].As< bool >( ) )
		{
		      try{
				m_networkProvider = new ServerNetworkProvider( this );
		       }catch(...){
	               cerr << "Unable to allocate the Requested Memory for ServerNetworkProvider\n";
	               delete m_networkProvider;
	               cerr << abort();
		}
		}
		else
		{
			try{
 	 		m_networkProvider = new ClientNetworkProvider( this );
			}catch(...){
	               cerr << "Unable to allocate the Requested Memory ClientNetworkProvider\n";
	               delete m_networkProvider;
	               cerr << abort();
		}
		}

		m_networkProvider->Initialize( configuration );
	}

	void NetworkSystem::PushMessage( const std::string& componentId, const std::string& message, AnyType::AnyTypeMap parameters )
	{
		try{
		m_networkProvider->PushMessage( componentId, message, parameters );
		}catch(...){
			cerr<< "API Faliled,to make a API call to NetworkSystem::PushMessage";
			throw;
		}
		
	}

	void NetworkSystem::Update( const float& deltaMilliseconds )
	{
		try{
			m_networkProvider->Update( deltaMilliseconds );
		}catch(...){
		cerr<< "API Faliled,to make a API call to NetworkSystem::Update";
		throw;
		}
	}

	void NetworkSystem::Message( const std::string& message, AnyType::AnyTypeMap parameters )
	{
		if( m_networkProvider )
		{
			try{
			m_networkProvider->Message( message, parameters );
			}catch(...){
				cerr<< "API Faliled,to make a API call to NetworkSystem::Message";
				throw; 
			}
		}
	}

	void NetworkSystem::MessageComponent( const std::string& componentName, const std::string& message, AnyType::AnyTypeMap parameters )
	{
		try{
			m_scene->MessageComponent( componentName, message, parameters );
		}catch(...){
			cerr << "API Faliled,to make a API call to NetworkSystem::MessageComponent";
			throw;
		}
	}
}
