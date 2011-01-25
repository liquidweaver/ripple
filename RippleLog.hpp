#ifndef RIPPLELOG_H
#define RIPPLELOG_H
#include "RippleUser.hpp"
#include "RippleDefines.hpp"

class RippleLog {
public:
	RippleLog( const RippleUser& creator )

	string Subject() const {
		return body.substr( 0, body.find_first_of( "\r\n" ) );
	}

	int log_id;
	RIPPLE_LOG_FLAVOR flavor;
	string body;
	int user_id;
	std:tm created_date;
}

namespace soci
{
template<> struct type_conversion<RippleLog>
{
	typedef values base_type;
    static void from_base(values const& v, indicator /* ind */, RippleLog& p) {
	 	if ( indicator == i_null )
			throw logic_error( "Cannot load log entry: database reports NULL" );
	 	p.log_id = v.get<int>("log_id");
		p.flavor = static_cast<RIPPLE_LOG_FLAVOR>( v.get<int>("flavor") );
		p.body = v.get<string>("body");
		p.created_date = v.get<std::tm>("created_date");
		p.user_id = v.get<int>("user_id");
    }

    static void to_base(const RippleLog& p, values& v, indicator& ind) {
	 	v.set("log_id", p.log_id, p.log_id == -1 ? i_null : i_ok );
		v.set("flavor", static_cast<int>( p.flavor ) );
		v.set("body", p.body );
		v.set("created_date", p.created_date );
		v.set("user_id", p.user_id );

		ind = i_ok;
    }
};
}

#endif //RIPPLELOG_H
