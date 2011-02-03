#ifndef RIPPLELOG_H
#define RIPPLELOG_H
#include "RippleUser.hpp"
#include "RippleDefines.hpp"
#include <time.h>

class RippleLog {
public:
	RippleLog( const RippleUser& creator, const RippleTask& task, RIPPLE_LOG_FLAVOR flavor, const string& subject_and_body ) 
	: log_id( -1 ), user_id( creator.user_id ), task_id( task.task_id ), flavor( flavor ), body( subject_and_body ) {
		created_date = time( NULL );
	}
	RippleLog() : log_id( -1 ), created_date( -1 ) { }

	string Subject() const {
		size_t pos = body.find_first_of( "\r\n" );
		if ( pos == string::npos )
			return body;
		return body.substr( 0, pos );
	}

	int log_id;
	RIPPLE_LOG_FLAVOR flavor;
	string body;
	int user_id;
	int task_id;
	time_t created_date;
};

namespace soci
{
template<> struct type_conversion<RippleLog>
{
	typedef values base_type;
    static void from_base(values const& v, indicator ind, RippleLog& p) {
	 	if ( ind == i_null )
			throw logic_error( "Cannot load log entry: database reports NULL" );
		std:tm created_tm;
	 	p.log_id = v.get<int>("log_id");
		p.flavor = static_cast<RIPPLE_LOG_FLAVOR>( v.get<int>("flavor") );
		p.body = v.get<string>("body");
		p.created_date  = v.get<int>("created_date");
		p.user_id = v.get<int>("user_id");
		p.task_id = v.get<int>("task_id");
    }

    static void to_base(const RippleLog& p, values& v, indicator& ind) {
	 	v.set("log_id", p.log_id, p.log_id == -1 ? i_null : i_ok );
		v.set("flavor", static_cast<int>( p.flavor ) );
		v.set("body", p.body );
		v.set("created_date", p.created_date );
		v.set("user_id", p.user_id );
		v.set("task_id", p.task_id );

		ind = i_ok;
    }
};
}

#endif //RIPPLELOG_H
