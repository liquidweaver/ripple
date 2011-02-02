#ifndef RIPPLEUSER_H
#define RIPPLEUSER_H
#include <soci/soci.h>
#include <string>

using namespace std;

class RippleUser {
public:
	RippleUser( string name, string email )
	: user_id( -1 ), name( name ), email( email ) { }
	RippleUser() : user_id( -1 ), name( "UNSET" ), email( "UNSET" ) { }
	int user_id;
	string name;
	string email;
};

namespace soci
{
template<> struct type_conversion<RippleUser>
{
    typedef values base_type;
    static void from_base(values const& v, indicator /* ind */, RippleUser& p) {
	 	p.user_id = v.get<int>("user_id");
		p.name = v.get<std::string>("name");
		p.email = v.get<std::string>("email");
    }

    static void to_base(const RippleUser& p, values& v, indicator& ind) {
	 	v.set("user_id", p.user_id, p.user_id == -1 ? i_null: i_ok );
	 	v.set("name", p.name );
	 	v.set("email", p.email );

		ind = i_ok;
    }
};
}

#endif //RIPPLEUSER_H
