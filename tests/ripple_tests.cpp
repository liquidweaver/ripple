#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ripple_tests
#include "../Ripple.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( RippleTests )

BOOST_AUTO_TEST_CASE( user_creation_and_deletion ) {
	Ripple* ripple = Ripple::Instance();
	RippleUser ru( "Joshua Weaver", "josh@metropark.com" );
	ru.password = "password";
	ripple->InsertUser( ru );

	BOOST_REQUIRE_GT( ru.user_id, 0 );
	RippleUser ru2;
	ripple->GetUser( ru.user_id, ru2 );
	BOOST_CHECK_EQUAL( ru2.name, "Joshua Weaver" );

	ripple->DeleteUser( ru2 );
	BOOST_REQUIRE_THROW( ripple->DeleteUser( ru2 ), RippleException );

	BOOST_REQUIRE_THROW( ripple->GetUser( ru2.user_id, ru2 ), RippleException );
	Ripple::Release();
}

BOOST_AUTO_TEST_CASE( task_creation ) {
	Ripple* ripple = Ripple::Instance();
	RippleUser ru( "Test User", "test@exampledomain.com" );
	ru.password = "password";
	ripple->InsertUser( ru );
	BOOST_REQUIRE_GT( ru.user_id, 0 );

	RippleUser ru2( "Test User 2", "test2@exampledomain.com" );
	ru2.password = "password";
	ripple->InsertUser( ru2 );
	BOOST_REQUIRE_GT( ru2.user_id, 0 );

	RippleTask rt = ripple->CreateTask( ru, "This is a test task\nOmg sweetness" );
	BOOST_REQUIRE_GT( rt.task_id, 0 );
	BOOST_REQUIRE_EQUAL( rt.stakeholder, ru.user_id );
	BOOST_REQUIRE_EQUAL( rt.assigned, ru.user_id );

	BOOST_CHECK_NO_THROW( ripple->ForwardTask( rt, ru, ru2, "I need you to do this." ) );
	vector<int> logs;
	ripple->GetLogsForTask( rt, logs );
	BOOST_REQUIRE_EQUAL( logs.size(), 2 );

	RippleLog log;
	ripple->GetLog( logs[0], log );
	BOOST_CHECK_EQUAL( log.Subject(), "This is a test task" );

	//Make sure time is reasonably accurate...5 seconds is a crazy amount of error margin
	//It should essentially be time(NULL)
	BOOST_CHECK_LT( time( NULL ) - log.created_date, 5 );

	ripple->GetLog( logs[1], log );
	BOOST_CHECK_EQUAL( log.body, "I need you to do this." );
	BOOST_CHECK_EQUAL( log.Subject(), "I need you to do this." );

	Ripple::Release();
}

BOOST_AUTO_TEST_SUITE_END()
