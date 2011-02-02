#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ripple_tests
#include "../Ripple.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( RippleTests )

BOOST_AUTO_TEST_CASE( user_creation_and_deletion ) {
	Ripple ripple;
	RippleUser ru( "Joshua Weaver", "josh@metropark.com" );
	ripple.InsertUser( ru );

	BOOST_REQUIRE_GT( ru.user_id, 0 );
	RippleUser ru2;
	ripple.GetUser( ru.user_id, ru2 );
	BOOST_CHECK_EQUAL( ru2.name, "Joshua Weaver" );
}

BOOST_AUTO_TEST_SUITE_END()
