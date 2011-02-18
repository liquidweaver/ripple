#ifndef RIPPLELOG_H
#define RIPPLELOG_H
#include "RippleUser.hpp"
#include "RippleDefines.hpp"
#include <time.h>

#define MAX_SUBJECT_LENGTH 30

class RippleLog {
public:
	RippleLog( const RippleUser& creator, const RippleTask& task, RIPPLE_LOG_FLAVOR flavor, const string& description ) 
	: log_id( -1 ), user_id( creator.user_id ), task_id( task.task_id ), flavor( flavor ), description( description ) {
		created_date = time( NULL );
	}
	RippleLog() : log_id( -1 ), created_date( -1 ) { }

	string Subject() const {
		size_t pos = EndOfSubject();
		if ( pos == string::npos )
			return description;
		return description.substr( 0, pos );
	}

	string Body() const {
		size_t pos = EndOfSubject();
		if ( pos == string::npos )
			return string( "" );

		while ( !isgraph( description[pos] ) ) {
			++pos;
			if ( pos == description.size() )
				return string( "" );
		}

		return description.substr( pos, description.size()  - 1);
	}



	int log_id;
	RIPPLE_LOG_FLAVOR flavor;
	string description;
	int user_id;
	int task_id;
	time_t created_date;

private:
	size_t EndOfSubject() const {
		size_t pos = description.find_first_of( "\r\n" );
		if ( 	( pos == string::npos && description.size() > MAX_SUBJECT_LENGTH ) 
				|| ( pos != string::npos && pos > MAX_SUBJECT_LENGTH - 1 ) ) {
			size_t alt_pos = description.find_last_of( " \r\n", MAX_SUBJECT_LENGTH - 1 );
			
			return alt_pos == string::npos ? MAX_SUBJECT_LENGTH - 1 : alt_pos;
		}

		return pos;
	}
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
		p.description = v.get<string>("description");
		p.created_date  = v.get<int>("created_date");
		p.user_id = v.get<int>("user_id");
		p.task_id = v.get<int>("task_id");
    }

    static void to_base(const RippleLog& p, values& v, indicator& ind) {
	 	v.set("log_id", p.log_id, p.log_id == -1 ? i_null : i_ok );
		v.set("flavor", static_cast<int>( p.flavor ) );
		v.set("description", p.description );
		v.set("created_date", p.created_date );
		v.set("user_id", p.user_id );
		v.set("task_id", p.task_id );

		ind = i_ok;
    }
};
}

#endif //RIPPLELOG_H
