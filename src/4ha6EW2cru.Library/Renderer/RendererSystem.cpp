#include "RendererSystem.h"

#include "RendererSystemScene.h"
#include "Color.hpp"
using namespace Ogre;

#include "../Management/Management.h"
#include "../Events/Event.h"
using namespace Events;

#include "../Logging/Logger.h"
using namespace Logging;

#include "../IO/BadArchiveFactory.h"
using namespace IO;

#include "Line3d.h"

#include "../Maths/MathVector3.hpp"
using namespace Maths;

namespace Renderer
{

	RendererSystem::~RendererSystem( )
	{	
		Ogre::WindowEventUtilities::removeWindowEventListener( m_window, this );

		if ( m_root != 0 )
		{
			m_root->shutdown( );
			delete m_root;
			m_root = 0;
		}
	}

	void RendererSystem::Initialize( )
	{
		if ( m_configuration == 0 )
		{
			NullReferenceException e( "RenderSystem::Initialize - The Configuration is NULL" );
			Logger::Fatal( e.what( ) );
			throw e;
		}

		bool defaultFullScreen = false;
		int defaultWidth = 640;
		int defaultHeight = 480;
		int defaultDepth = 32;
		bool defaultVsync = true;
		std::string defaultWindowTitle = "Interactive View";

		m_configuration->SetDefault( m_configSectionName, "fullscreen", defaultFullScreen );
		m_configuration->SetDefault( m_configSectionName, "width", defaultWidth );
		m_configuration->SetDefault( m_configSectionName, "height", defaultHeight );
		m_configuration->SetDefault( m_configSectionName, "depth", defaultDepth );
		m_configuration->SetDefault( m_configSectionName, "window_title", defaultWindowTitle );
		m_configuration->SetDefault( m_configSectionName, "vsync", defaultVsync );

		m_root = new Root( );

	#ifdef _DEBUG
		m_root->loadPlugin( "RenderSystem_Direct3D9_d" );
	#else
		m_root->loadPlugin( "RenderSystem_Direct3D9" );

		Ogre::LogManager::getSingletonPtr( )->destroyLog( Ogre::LogManager::getSingletonPtr( )->getDefaultLog( ) );
		Ogre::LogManager::getSingletonPtr( )->createLog( "default", true, false, true );
	#endif // _DEBUG	

		//_badFactory = new BadArchiveFactory( );
		ArchiveManager::getSingletonPtr( )->addArchiveFactory( new BadArchiveFactory( ) );

		ResourceGroupManager::getSingleton( ).addResourceLocation( "/", "BAD" );
		ResourceGroupManager::getSingleton( ).initialiseAllResourceGroups( );

		RenderSystemList *renderSystems = m_root->getAvailableRenderers( );
		RenderSystemList::iterator renderSystemIterator = renderSystems->begin( );

		m_root->setRenderSystem( *renderSystemIterator );

		std::stringstream videoModeDesc;
		videoModeDesc << m_configuration->Find( m_configSectionName, "width" ).As< int >( ) << " x " << m_configuration->Find( m_configSectionName, "height" ).As< int >( ) << " @ " << defaultDepth << "-bit colour";
		( *renderSystemIterator )->setConfigOption( "Video Mode", videoModeDesc.str( ) );
		( *renderSystemIterator )->setConfigOption( "Full Screen", m_configuration->Find( m_configSectionName, "fullscreen" ).As< bool >( ) ? "Yes" : "No" );
		( *renderSystemIterator )->setConfigOption( "VSync", m_configuration->Find( m_configSectionName, "vsync" ).As< bool >( ) ? "Yes" : "No" );

		m_root->initialise( false );

		try
		{
			this->CreateRenderWindow(
				m_configuration->Find( m_configSectionName, "window_title" ).As< std::string >( ), 
				m_configuration->Find( m_configSectionName, "width" ).As< int >( ), 
				m_configuration->Find( m_configSectionName, "height" ).As< int >( ),
				m_configuration->Find( m_configSectionName, "fullscreen" ).As< bool >( )
				);
		}
		catch( Exception e )
		{
			if ( e.getNumber( ) == Exception::ERR_RENDERINGAPI_ERROR )
			{
				m_configuration->Set( m_configSectionName, "fullscreen", defaultFullScreen );
				m_configuration->Set( m_configSectionName, "width", defaultWidth );
				m_configuration->Set( m_configSectionName, "height", defaultHeight );
				m_configuration->Set( m_configSectionName, "depth", defaultDepth );
				m_configuration->Set( m_configSectionName, "window_title", defaultWindowTitle );

				Management::GetInstance( )->GetPlatformManager( )->CloseWindow( );
			}
		}

		Ogre::WindowEventUtilities::addWindowEventListener( m_window, this );

		m_attributes[ "availableVideoModes" ] = this->GetVideoModes( );

		m_sceneManager = m_root->createSceneManager( ST_GENERIC, "default" );

		Camera* camera = m_sceneManager->createCamera( "default" );
		camera->setPosition( Vector3( 0, 0, 0 ) );
		camera->lookAt( Vector3( 0, 0, 0 ) );
		camera->setNearClipDistance( 1.0f );
		camera->setFarClipDistance( 500.0f );

		Viewport* viewPort = m_window->addViewport( camera );
		viewPort->setBackgroundColour( ColourValue( 0, 0, 0 ) );

		camera->setAspectRatio(
			Real( viewPort->getActualWidth( )) / Real( viewPort->getActualHeight( ) )
			);

		m_root->renderOneFrame( );

		Management::GetInstance( )->GetServiceManager( )->RegisterService( this );

		//CompositorManager::getSingletonPtr( )->addCompositor( m_sceneManager->getCurrentViewport( ), "HDR" );
		//CompositorManager::getSingletonPtr( )->setCompositorEnabled( m_sceneManager->getCurrentViewport( ), "HDR", true );
	}

	void RendererSystem::Update( const float& deltaMilliseconds )
	{
		m_root->renderOneFrame( );
	}

	void RendererSystem::SetAttribute( const std::string& name, AnyType value )
	{
		m_attributes[ name ] = value;

		if ( name == "activeCamera" )
		{
			std::string cameraName = value.As< std::string >( );
			Camera* camera = m_sceneManager->getCamera( cameraName );
			m_sceneManager->getCurrentViewport( )->setCamera( camera );
		}

		if ( name == "ambientColor" )
		{
			Color color = value.As< Renderer::Color >( );
			ColourValue colorValue( color.Red, color.Green, color.Blue );

			m_sceneManager->setAmbientLight( colorValue );
		}

		if ( name == "backgroundColor" )
		{
			Color color = value.As< Renderer::Color >( );
			ColourValue colorValue( color.Red, color.Green, color.Blue );

			m_sceneManager->getCurrentViewport( )->setBackgroundColour( colorValue );
		}

		if ( name == "farClip" )
		{
			float farClip = value.As< float >( );

			if ( m_sceneManager->isSkyBoxEnabled( ) )
			{
				SceneManager::SkyBoxGenParameters skyBoxParameters = m_sceneManager->getSkyBoxGenParameters( ); 
				m_sceneManager->setSkyBox( true, m_skyBoxMaterial, 350 );
			}

			m_sceneManager->getCurrentViewport( )->getCamera( )->setFarClipDistance( farClip );
		}

		if( name == "fog" )
		{
			AnyType::AnyTypeMap parameters = value.As< AnyType::AnyTypeMap >( );
			Color color = parameters[ "color" ].As< Renderer::Color >( );

			m_sceneManager->setFog( 
				FOG_LINEAR, 
				ColourValue( color.Red, color.Green, color.Blue ),
				0.001000, 
				parameters[ "linearStart" ].As< float >( ), 
				parameters[ "linearEnd" ].As< float >( ) 
				);				

			/*Ogre::Plane skyPlane;
			skyPlane.d = 50;
			skyPlane.normal = Vector3::NEGATIVE_UNIT_Y;

			m_sceneManager->setSkyPlane( true, skyPlane, "24-Default", 8, 4, true, 0, 4, 4 );*/
		}

		if ( name == "skyBox" )
		{
			AnyType::AnyTypeMap parameters = value.As< AnyType::AnyTypeMap >( );

			std::string material = parameters[ "material" ].As< std::string >( );

			if ( material.empty( ) )
			{
				m_sceneManager->setSkyBox( false, "" );
			}
			else
			{
				m_skyBoxMaterial = material;

				m_sceneManager->setSkyBox( 
					true, 
					m_skyBoxMaterial, 
					parameters[ "distance" ].As< float >( ) );
			}
		}
	}

	ISystemScene* RendererSystem::CreateScene( )
	{
		m_scene = new RendererSystemScene( m_sceneManager );
		return m_scene;
	}

	void RendererSystem::windowClosed( RenderWindow* rw )
	{
		Management::GetInstance( )->GetEventManager( )->QueueEvent( new Event( GAME_QUIT ) );
	}

	std::vector< std::string > RendererSystem::GetVideoModes( ) const
	{
		std::vector< std::string > availableDisplayModes;
		ConfigOptionMap options = m_root->getRenderSystem( )->getConfigOptions( );

		for( ConfigOptionMap::iterator cm = options.begin( ); cm != options.end( ); ++cm )
		{
			if ( ( *cm ).first == "Video Mode" )
			{
				StringVector possibleModes = ( *cm ).second.possibleValues;

				for( StringVector::iterator i = possibleModes.begin( ); i != possibleModes.end( ); ++i )
				{
					std::stringstream currentColorDepth;
					currentColorDepth << m_window->getColourDepth( );

					int result = ( *i ).find( currentColorDepth.str( ) );

					if ( result > -1 )
					{
						std::string mode = ( *i ).substr( 0, ( *i ).find( " @ " ) );
						availableDisplayModes.push_back( mode );
					}
				}
			}
		}

		return availableDisplayModes;
	}

		

	void RendererSystem::CreateRenderWindow( const std::string& windowTitle, int width, int height, bool fullScreen )
	{
		Management::GetInstance( )->GetPlatformManager( )->CreateInteractiveWindow( windowTitle, width, height, fullScreen );
		
		NameValuePairList params;
		params[ "externalWindowHandle" ] = StringConverter::toString( ( int ) Management::GetInstance( )->GetPlatformManager( )->GetWindowId( ) );
		params[ "vsync" ] = m_configuration->Find( m_configSectionName, "vsync" ).As< bool >( ) ? "true" : "false";

		m_window = m_root->createRenderWindow( windowTitle, width, height, fullScreen, &params ); 
	}

	AnyType::AnyTypeMap RendererSystem::Execute( const std::string& message, AnyType::AnyTypeMap& parameters )
	{
		AnyType::AnyTypeMap results;

		if ( message == System::Messages::PlayAnimation )
		{
			IRendererSystemComponent* component = m_scene->GetComponent( parameters[ "entityName" ].As< std::string >( ) );
			
			component->PlayAnimation( 
				parameters[ "animationName" ].As< std::string >( ),
				parameters[ "loopAnimation" ].As< bool >( )
				);
		}

		if ( message == "getRenderWindow" )
		{
			results[ "renderWindow" ] = m_window;
		}

		if( message == "getAvailableVideoModes" )
		{
			results[ "availableVideoModes" ] = this->GetVideoModes( );
		}

		if ( message == "changeResolution" )
		{
			m_window->setFullscreen(  
				parameters[ "fullScreen" ].As< bool >( ),
				parameters[ "width" ].As< int >( ),
				parameters[ "height" ].As< int >( )
				);
		}

		if ( message == "drawLine" )
		{
			Line3D* line = new Line3D( );
			line->drawLine( 
				parameters[ "origin" ].As< MathVector3 >( ).AsOgreVector3( ), 
				parameters[ "destination" ].As< MathVector3 >( ).AsOgreVector3( ) 
				);

			SceneNode* lineNode = m_sceneManager->createSceneNode( );
			lineNode->attachObject( line );
			m_sceneManager->getRootSceneNode( )->addChild( lineNode );

		}

		if ( message == "screenShot" )
		{
			m_window->writeContentsToFile( "C:\\Users\\NK\\Desktop\\output.png" );
		}

		return results;
	}
}