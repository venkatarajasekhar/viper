/*!
*  @company Black Art Studios
*  @author Nicholas Kostelnik
*  @file   NetworkFacade.h
*  @date   2009/07/06
*/
#pragma once
#ifndef NETWORKFACADE_H
#define NETWORKFACADE_H

#include "INetworkFacade.h"

#include <luabind/luabind.hpp>

#include "System/ISystemComponent.hpp"

namespace Script
{
	/*! 
	 *  A Facade interface to interact with the Network System from Script
	 */
	class NetworkFacade : public INetworkFacade
	{

	public:

		/*! Default Destructor
		 *
		 *  @return ()
		 */
		~NetworkFacade( ) { };


		/*! Default Constructor
		*
		* @return (  )
		*/
		NetworkFacade( ISystemComponent* component )
			: m_component( component )
		{

		}

		/*! Registers the Script functions with the given state
		*
		* @param[in] lua_State * state
		* @return ( void )
		*/
		static void RegisterFunctions( lua_State* state );


		/*! Requests the Game connects to the specified server
		*
		* @param[in] const std::string & serverAddress
		* @param[in] const int & port
		* @return ( void )
		*/
		void Connect( const std::string& hostAddress, const int& port );


	private:

		NetworkFacade( const NetworkFacade & copy ) { };
		NetworkFacade & operator = ( const NetworkFacade & copy ) { return *this; };

		ISystemComponent* m_component;
		
	};
};

#endif