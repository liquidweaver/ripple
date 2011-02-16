#ifndef RIPPLETASK_H
#define RIPPLETASK_H
#include "RippleUser.hpp"
#include "RippleDefines.hpp"

class RippleTask {
public:
	RippleTask()
		: task_id( -1 ), stakeholder( -1 ), assigned( -1 ), start_date( -1 ),
			due_date( -1 ), state( RTS_OPEN ), parent_task( -1 ) { }

	RippleTask( const RippleUser& stakeholder )
		: task_id( -1 ), stakeholder( stakeholder.user_id ), assigned( stakeholder.user_id ), start_date( -1 ),
			due_date( -1 ), state( RTS_OPEN ), parent_task( -1 ) { }

	bool IsStakeHolder( const RippleUser& user ) const {
		return user.user_id == stakeholder;
	}

	bool IsAssigned( const RippleUser& user ) const {
		return user.user_id == assigned;
	}

	int task_id; 
	int stakeholder; 
	int assigned; 
	std::time_t start_date; 
	std::time_t due_date; 
	RIPPLE_TASK_STATE state; 
	int parent_task; 
};


namespace soci
{
template<> struct type_conversion<RippleTask>
{
	typedef values base_type;
	static void from_base(values const& v, indicator /* ind */, RippleTask& p) {
		p.task_id = v.get<int>("task_id");
		p.stakeholder = v.get<int>("stakeholder");
		p.assigned = v.get<int>("assigned");
		v.get_indicator("start_date") == i_null ? p.start_date = -1 : p.start_date = v.get<int>("start_date");
		v.get_indicator("due_date") == i_null ? p.due_date = -1 : p.due_date = v.get<int>("due_date");
		v.get_indicator("parent_task") == i_null ? p.parent_task = -1 : p.parent_task = v.get<int>("parent_task");
		p.state = static_cast<RIPPLE_TASK_STATE>( v.get<int>("state") );
	}

	static void to_base(const RippleTask& p, values& v, indicator& ind) {
		v.set("task_id", p.task_id, p.task_id == -1 ? i_null : i_ok );
		v.set("stakeholder", p.stakeholder );
		v.set("assigned", p.assigned );
		v.set("start_date", p.start_date, p.start_date == -1 ? i_null : i_ok );
		v.set("due_date", p.due_date, p.due_date == -1 ? i_null : i_ok );
		v.set("parent_task", p.task_id, p.task_id == -1 ? i_null : i_ok );
		v.set("state", static_cast<int>( p.state ) );

		ind = i_ok;
	}
};
}
#endif //RIPPLETASK_H
