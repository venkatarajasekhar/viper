#include "BadArchiveFixture.h"

#include "IO/FileManager.h"
#include "Logging/Logger.h"
#include "IO/BadArchive.h"

#include "Exceptions/FileNotFoundException.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( BadArchiveFixture );

void BadArchiveFixture::setUp( )
{
	Logger::Initialize( );
	FileManager::Initialize( );
}

void BadArchiveFixture::tearDown( )
{
	FileManager::GetInstance( )->Release( );
	Logger::GetInstance( )->Release( );
}

void BadArchiveFixture::Should_Load( )
{

}

void BadArchiveFixture::Should_Unload( )
{

}

void BadArchiveFixture::Should_Return_True_On_Case_Sensitive( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	
	CPPUNIT_ASSERT( archive->isCaseSensitive( )	);
	
	delete archive;
}

void BadArchiveFixture::Should_Return_DataStreamPtr_Given_File_Found( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	Ogre::DataStreamPtr stream = archive->open( "Test.txt" );

	CPPUNIT_ASSERT( !stream.isNull( ) );

	delete archive;
}

void BadArchiveFixture::Sould_Return_Throw_Given_File_Not_Found( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );

	CPPUNIT_ASSERT_THROW( archive->open( "blah" ), FileNotFoundException );

	delete archive;
}

void BadArchiveFixture::Should_Return_False_From_Exists_Given_NON_Existing_File( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	bool result = archive->exists( "blah" );

	CPPUNIT_ASSERT( !result );

	delete archive;
}

void BadArchiveFixture::Should_Return_True_From_Exists_Given_File_Exists( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	bool result = archive->exists( "Test.txt" );

	CPPUNIT_ASSERT( result );

	delete archive;
}

void BadArchiveFixture::Should_Return_Empty_StringVectorPtr_On_List( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	Ogre::StringVectorPtr files = archive->list( "*" );

	CPPUNIT_ASSERT( !files.isNull( ) );

	delete archive;
}

void BadArchiveFixture::Should_Return_Empty_FileInfoPtr_On_ListFileInfo( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	Ogre::FileInfoListPtr info = archive->listFileInfo( false, false );

	CPPUNIT_ASSERT( !info.isNull( ) );

	delete archive;
}

void BadArchiveFixture::Should_Return_Empty_StringVectorPtr_On_Find( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	Ogre::StringVectorPtr files = archive->find( "*" );

	CPPUNIT_ASSERT( !files.isNull( ) );

	delete archive;
}

void BadArchiveFixture::Should_Return_Empty_FileInfoPtr_On_FindFileInfo( )
{
	BadArchive* archive = new BadArchive( "../game/test/Test.bad", "BAD" );
	Ogre::FileInfoListPtr info = archive->findFileInfo( "*", false, false );

	CPPUNIT_ASSERT( !info.isNull( ) );

	delete archive;
}