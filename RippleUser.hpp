#ifndef RIPPLEUSER_H
#define RIPPLEUSER_H
#include <soci/soci.h>
#include <string>
#include "RippleException.hpp"

using namespace std;

class RippleUser {
public:
	RippleUser( string name, string email )
	: user_id( -1 ), name( name ), email( email ) { }
	RippleUser() : user_id( -1 ), name( "UNSET" ), email( "UNSET" ), password( "UNSET" ) { }
	int user_id;
	string name;
	string email;
	string password;
	string avatar_file;
};

namespace soci
{
template<> struct type_conversion<RippleUser>
{
    typedef values base_type;
    static void from_base(values const& v, indicator ind, RippleUser& p) {
		 if ( ind == i_null )
			 throw RippleException( "User not found." );
		 p.user_id = v.get<int>("user_id");
		 p.name = v.get<std::string>("name");
		 p.email = v.get<std::string>("email");
		 p.password = v.get<std::string>("password");
		 v.get_indicator("avatar_file") == i_null ? p.avatar_file = "" : p.avatar_file = v.get<string>("avatar_file");
    }

    static void to_base(const RippleUser& p, values& v, indicator& ind) {
	 	v.set("user_id", p.user_id, p.user_id == -1 ? i_null: i_ok );
	 	v.set("name", p.name );
	 	v.set("email", p.email );
	 	v.set("password", p.password );
		v.set("avatar_file", p.avatar_file, p.avatar_file == "" ? i_null : i_ok );

		ind = i_ok;
    }
};
}

#endif //RIPPLEUSER_H
