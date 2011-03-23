#include "Ripple.hpp"
#include <algorithm> //std::find
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <iostream> //DEBUGGING
#include "restcomet/restcomet.h"
#include "JSON.hpp"

using namespace soci;

Ripple* Ripple::instance = NULL;
backend_factory const &backEnd = *soci::factory_sqlite3();
std::ostream& operator<<(std::ostream& out, const RippleUser& user ) {

	out << '{'
		<< '"' << "user_id" << "\":" << user.user_id << ','
		<< '"' << "name" << "\":\"" << JSON::escape( user.name ) << "\","
		<< '"' << "email" << "\":\"" << JSON::escape( user.email ) << '"';
	if ( user.avatar_file != "" )
		out << ",\"avatar_file" << "\":\"" << user.avatar_file << '"';

	out << '}';

	return out;

}

std::ostream& operator<<(std::ostream& out, const RippleLog& log ) {
	out << '{'
		<< '"' << "log_id" << "\":" << log.log_id << ','
		<< '"' << "flavor" << "\":" << log.flavor << ','
		<< '"' << "body" <<  "\":\"" << JSON::escape( log.Body() ) << '"' << ','
		<< '"' << "subject" <<  "\":\"" << JSON::escape( log.Subject() ) << '"' << ','
		<< '"' << "user_id" << "\":"  << log.user_id << ','
		<< '"' << "task_id" << "\":" << log.task_id << ','
		<< '"' << "created_date" << "\":\"" << JSON::rfc3339( log.created_date ) << '"';
		try { 
			RippleUser user;
			Ripple::instance->GetUser( log.user_id, user );
			out << ",\"" << "user_name\":\"" << JSON::escape( user.name ) << "\"";
			if ( user.avatar_file != "" ) 
				out << ",\"user_avatar\":\"" << JSON::escape( user.avatar_file ) << "\"";
		}
		catch ( ... ) { }
		out << '}';

	return out;
}

std::ostream& operator<<(std::ostream& out, const RippleTask& task ) {

	out << '{'
		<< '"' << "task_id" << "\":" << task.task_id << ','
		<< '"' << "stakeholder" << "\":" << task.stakeholder << ','
		<< '"' << "assigned" << "\":" << task.assigned << ','
		<< '"' << "start_date" << "\":\"" << JSON::rfc3339( task.start_date ) << "\","
		<< '"' << "due_date" << "\":\"" << JSON::rfc3339( task.due_date ) << "\","
		<< '"' << "state" << "\":"  << task.state << ','
		<< '"' << "parent_task" << "\":" << task.parent_task;
		
	try { 
		RippleUser stakeholder;
		Ripple::instance->GetUser( task.stakeholder, stakeholder );
		out << ",\"" << "stakeholder_name\":\"" << JSON::escape( stakeholder.name ) << "\"";
		if ( stakeholder.avatar_file != "" ) 
			out << ",\"stakeholder_avatar\":\"" << JSON::escape( stakeholder.avatar_file ) << "\"";
	}
	catch ( ... ) { }

	try { 
		RippleUser assigned;
		Ripple::instance->GetUser( task.assigned, assigned );
		out << ",\"" << "assigned_name\":\"" << JSON::escape( assigned.name ) << "\"";
		if ( assigned.avatar_file != "" ) 
			out << ",\"assigned_avatar\":\"" << JSON::escape( assigned.avatar_file ) << "\"";
	}
	catch ( ... ) { }

	vector<int> log_ids;

	Ripple::instance->GetLogsForTask( task, log_ids );

	if ( log_ids.size() > 0 ) {
		out << ",\"logs\":[";

		for( vector<int>::const_iterator log_id = log_ids.begin(); log_id != log_ids.end(); ++log_id ) {
			RippleLog log;
			Ripple::instance->GetLog( *log_id, log );
			out << log;
			if ( log_id + 1 != log_ids.end() )
				out << ',';
		}

		out << ']';
	}
	out << '}';

	return out;
}
  

Ripple* Ripple::Instance() {
	if ( instance == NULL )
		instance = new Ripple();
	
	return instance;
}

void Ripple::Release() {
	if ( instance != NULL )
		delete instance;
	
	instance = NULL;

}

Ripple::Ripple() : pool( DB_POOL_SIZE ) {
	for ( size_t i = 0; i != DB_POOL_SIZE; ++i ) {
		session& sql = pool.at(i);
		sql.open( backEnd, "ripple.db"  );
		sql << "PRAGMA foreign_keys = ON";
	}

	KnownFlavors.push_back( RLF_CREATED );
	KnownFlavors.push_back( RLF_NOTE );
	KnownFlavors.push_back( RLF_FORWARDED );
	KnownFlavors.push_back( RLF_FEEDBACK );
	KnownFlavors.push_back( RLF_DECLINED );
	KnownFlavors.push_back( RLF_ACCEPTED );
	KnownFlavors.push_back( RLF_STARTED );
	KnownFlavors.push_back( RLF_COMPLETED );
	KnownFlavors.push_back( RLF_REOPENED );
	KnownFlavors.push_back( RLF_CLOSED );
	KnownFlavors.push_back( RLF_CANCELED );

	rc = rc::restcomet::Instance( EVENTS_PORT );
}

void Ripple::InsertUser( RippleUser& ru ) {
	assert( ru.user_id == -1 );

	if ( ru.email == "UNSET" || ru.email == "" )
		throw RippleException( "Email must be specified." );
	if ( ru.name == "UNSET" || ru.name == "" )
		throw RippleException( "Name must be specified." );
	if ( ru.password == "UNSET" || ru.password == "" )
		throw RippleException( "Password must be specified." );
	
	soci::session sql( pool );

	sql << "INSERT INTO users VALUES (:user_id, :name, :email, :password, :avatar_file)",
		 use( static_cast<const RippleUser&>( ru ) );

	sql << "SELECT last_insert_rowid()", into( ru.user_id );

	if ( !sql.got_data() )
		throw RippleException( "User could not be inserted." );
}

void Ripple::GetUser( int user_id, RippleUser& user ) {
	indicator ind;
	if ( user_id < 1 )
		throw RippleException( "invalid user_id" );
	soci::session sql( pool );
	sql << "SELECT * FROM users WHERE user_id=:user_id",
		 into( user, ind ), use( user_id );

	if ( !sql.got_data() ) 
		throw RippleException( "User not found." );
}

void Ripple::GetUsers( vector<int>& user_ids ) {
	user_ids.clear();
	user_ids.resize( 500 );
	
	soci::session sql( pool );
	sql << "SELECT user_id FROM users",
		into( user_ids );
}

void Ripple::UpdateUser( const RippleUser& user ) {

	soci::session sql( pool );
	sql << "UPDATE users SET name=:name, email=:email," 
			<< "password=:password, avatar_file=:avatar_file WHERE user_id=:user_id",
		use( user );

}

RippleUser Ripple::GetUserFromEmailAndPassword( const string& email, const string& password ) {
	RippleUser ru;
	indicator ind;
	soci::session sql( pool );
	sql << "SELECT * FROM users WHERE email=:email AND password=:password",
				into(ru, ind ), use( email ), use( password );

	if ( !sql.got_data() )
		throw RippleException( "Invalid email or password" );

	return ru;
}

void Ripple::DeleteUser( RippleUser& user ) {
	DeleteUser( user.user_id );
	user.user_id = -1;
}

void Ripple::DeleteUser( int user_id ) {
	soci::session sql( pool );
	sql << "DELETE FROM users WHERE user_id=:user_id",
		use( user_id );

	int changes;
	sql << "SELECT changes()", into( changes );
	if ( changes == 0 )
		throw RippleException( "User not found." );
}

RippleTask Ripple::CreateTask( const RippleUser& ru, const string& description, std::time_t start, std::time_t due ) {
	if ( Blank( description ) )
		throw RippleException( "You must enter a description when creating a task." );
	soci::session sql( pool );
	transaction tr(sql);

	RippleTask rt( ru );
	rt.start_date = start;
	rt.due_date = due;
	InsertTask( rt, &sql );

	RippleLog rl( ru, rt, RLF_CREATED, description );
	InsertLog( rl, &sql );

	stringstream task_data, user_id;
	tr.commit();
	task_data << rt;
	user_id << rt.assigned;
	rc->SubmitEvent( user_id.str(), task_data.str() );
	if ( rt.assigned != rt.stakeholder ) {
		user_id << rt.stakeholder;
		rc->SubmitEvent( user_id.str(), task_data.str() );
	}

	return rt;
}

void Ripple::GetTask( int task_id, RippleTask& task ) {
	assert( task_id > 0 );

	soci::session sql( pool );
	sql << "SELECT * FROM tasks WHERE task_id=:task_id",
		into( task ), use( task_id );
	
	if ( !sql.got_data() )
		throw RippleException( "Task not found." );
}

void Ripple::GetUsersTasks( int user_id, vector<int>& tasks ) {
	assert( user_id > 0 );
	tasks.resize( 200 );


	soci::session sql( pool );
	sql << "SELECT task_id FROM tasks WHERE ( assigned=:a_user_id OR stakeholder=:s_user_id ) AND state < :end",
		 into( tasks ), use( user_id ), use( user_id ), use( static_cast<int>( RTS_CLOSED ) );
}

void Ripple::GetLogsForTask( const RippleTask& task, vector<int>& logs ) {
	logs.clear();
	logs.resize( 500 );
	
	soci::session sql( pool );
	sql << "SELECT log_id FROM logs WHERE task_id=:task_id",
		into( logs ), use( task.task_id );
	
	if ( !sql.got_data() )
		throw RippleException( "No logs found. Does task exist?" );
}

void Ripple::GetLog( int log_id, RippleLog& log ) {

	soci::session sql( pool );
	sql << "SELECT * FROM logs WHERE log_id=:log_id",
			into( log ), use( log_id );

	if ( !sql.got_data() )
		throw RippleException( "Log not found." );

}

void Ripple::ReOpenTask( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_REOPENED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( Blank( reason ) )
		throw RippleException( "You must enter a reason when reopening a task." );
	int previously_assigned = task.assigned;
	task.assigned = task.stakeholder; //Reset assigned to stakeholder
	task.state = RTS_OPEN;
	RippleLog log( requestor, task, RLF_REOPENED, reason );
	UpdateTask( task, log, previously_assigned );
}

void Ripple::CancelTask( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_CANCELED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( Blank( reason ) )
		throw RippleException( "You must enter a reason when canceling a task." );
	int previously_assigned = task.assigned;
	task.assigned = task.stakeholder; //Reset assigned to stakeholder
	task.state = RTS_CANCELED;
	RippleLog log( requestor, task, RLF_CANCELED, reason );
	UpdateTask( task, log, previously_assigned );
}

void Ripple::AcceptTask( RippleTask& task, const RippleUser& requestor, string reason ) {
	string cannot = CheckAction( task, requestor, RLF_ACCEPTED );
	if ( cannot != "" )
		throw RippleException( cannot );

	task.state = RTS_ACCEPTED;
	RippleLog log( requestor, task, RLF_ACCEPTED, reason );
	UpdateTask( task,log );
}

void Ripple::ForwardTask( RippleTask& task, const RippleUser& requestor, const RippleUser& target, string reason ) {
	string cannot = CheckAction( task, requestor, RLF_FORWARDED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( requestor.user_id == target.user_id )
		throw RippleException( "you cannot forward a task to yourself" );

	int previously_assigned = task.assigned;
	task.assigned = target.user_id;
	RippleLog log( requestor, task, RLF_FORWARDED, reason );
	UpdateTask( task, log, previously_assigned );
}

void Ripple::RequestFeedback( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_FEEDBACK );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( Blank( reason ) )
		throw RippleException( "You must enter a reason when requesting feedback for a task." );
	int previously_assigned = task.assigned;
	task.assigned = GetLastAssigned( task );
	RippleLog log( requestor, task, RLF_FEEDBACK, reason );
	UpdateTask( task, log, previously_assigned );
}

void Ripple::DeclineTask( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_DECLINED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( Blank( reason ) )
		throw RippleException( "You must enter a reason when requesting declining a task." );
	int previously_assigned = task.assigned;
	task.assigned = GetLastAssigned( task );
	RippleLog log( requestor, task, RLF_DECLINED, reason );
	UpdateTask( task, log, previously_assigned );
}

void Ripple::StartTask( RippleTask& task, const RippleUser& requestor, string reason ) {
	string cannot = CheckAction( task, requestor, RLF_STARTED );
	if ( cannot != "" )
		throw RippleException( cannot );

	task.state = RTS_STARTED;
	RippleLog log( requestor, task, RLF_STARTED, reason );
	UpdateTask( task, log );
}

void Ripple::CompleteTask( RippleTask& task, const RippleUser& requestor, string reason ) {
	string cannot = CheckAction( task, requestor, RLF_COMPLETED );
	if ( cannot != "" )
		throw RippleException( cannot );

	int previously_assigned = task.assigned;
	try { //Pass this back to the previous if possible
		task.assigned = GetLastAssigned( task );
	}
	catch ( RippleException& e ) 
	{ }

	task.state = RTS_COMPLETED;
	RippleLog log( requestor, task, RLF_COMPLETED, reason );
	UpdateTask( task, log, previously_assigned );
}

void Ripple::CloseTask( RippleTask& task, const RippleUser& requestor, string reason ) {
	string cannot = CheckAction( task, requestor, RLF_CLOSED );
	if ( cannot != "" )
		throw RippleException( cannot );

	task.state = RTS_CLOSED;
	RippleLog log( requestor, task, RLF_CLOSED, reason );
	UpdateTask( task, log );
}

void Ripple::AddNoteToTask( const RippleTask& task, const RippleUser& requestor, const string& description ) {
	string cannot = CheckAction( task, requestor, RLF_NOTE );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( Blank( description ) )
		throw RippleException( "Note must have some content." );
	RippleLog log( requestor, task, RLF_NOTE, description );
	UpdateTask( task, log );
}

void Ripple::GetPossibleActions( const RippleTask& task, const RippleUser& requestor, map<RIPPLE_LOG_FLAVOR, string>& actions ) {
	actions.clear();

	for( vector<RIPPLE_LOG_FLAVOR>::const_iterator flavor = KnownFlavors.begin();
			flavor != KnownFlavors.end(); ++flavor ) {
		actions[*flavor] = CheckAction( task, requestor, *flavor );
	}
}

bool Ripple::Blank( const string& str ) {
	if ( str == "" )
		return true;

	if ( str.find_first_of( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`1234567890-=~!@#$%^&*()_+[];',./{}:\"<>?" ) == string::npos )
		return true;

	return false;
}

string Ripple::CheckAction( const RippleTask& task, const RippleUser& requestor, RIPPLE_LOG_FLAVOR flavor ) {
	if ( task.state == RTS_CLOSED || task.state == RTS_CANCELED ) 
		return string( "Task is finalized; no actions can be performed." );
	if ( !task.IsAssigned( requestor ) && !task.IsStakeHolder( requestor ) )
		return string( "You are neither assigned or the stakeholder of this task." );

	switch( flavor ) {
		case RLF_CREATED: //Never allowed; task already exists
			return string( "Task already exists." );
			break;
		case RLF_NOTE:
			if ( task.IsAssigned( requestor ) && ( task.state < RTS_ACCEPTED && task.state > RTS_STARTED ) )
				return string( "As assigned only, you can only enter notes if you have accepted or started the task." );
			break;
		case RLF_FORWARDED:
			if (!task.IsAssigned( requestor ) )
				return string( "You must be assigned to a task to forward it." );
			if ( task.state != RTS_OPEN )
				return string( "You cannot forward a task unless it is open." ); 
			break;
		case RLF_FEEDBACK:
			if (!task.IsAssigned( requestor ) )
				return string( "You must be assigned to a task to request feedback." );
			if ( task.state != RTS_OPEN )
				return string( "You cannot request feedback  for a task unless it is open." ); 
			try {
				GetLastAssigned( task );
			}
			catch ( exception& e ) {
				return e.what();
			}
			break;
		case RLF_DECLINED:
			if (!task.IsAssigned( requestor ) )
				return string( "You must be assigned to a task to decline it." );
			if ( task.state != RTS_OPEN )
				return string( "You cannot decline a task unless it is open." ); 
			try {
				GetLastAssigned( task );
			}
			catch ( exception& e ) {
				return e.what();
			}
			break;
		case RLF_ACCEPTED:
			if (!task.IsAssigned( requestor ) )
				return string( "You must be assigned to a task to accept it." );
			if ( task.IsStakeHolder( requestor ) )
				return string( "Acceptance by the stakeholder is implicit." );
			if ( task.state != RTS_OPEN )
				return string( "You cannot accept a task unless it is open." ); 
			break;
		case RLF_STARTED:
			if (!task.IsAssigned( requestor ) )
				return string( "You must be assigned to a task to start it." );
			if ( task.IsStakeHolder( requestor ) )
				return string( "Acceptance by the stakeholder is implicit." );
			if ( task.state != RTS_OPEN && task.state != RTS_ACCEPTED )
				return string( "You cannot start a task unless it is open or accepted." ); 
			break;
		case RLF_COMPLETED:
			if (!task.IsAssigned( requestor ) )
				return string( "You must be assigned to a task to complete it." );
			if ( task.IsStakeHolder( requestor ) )
				return string( "The stakeholder just closes a task to complete it." );
			else if ( task.IsAssigned( requestor ) && task.state < RTS_ACCEPTED )
				return string( "You must accept the task first to complete it." );
			if ( !(task.state == RTS_OPEN || task.state == RTS_ACCEPTED || 
						task.state == RTS_STARTED || task.state == RTS_COMPLETED ) )
				return string( "You cannot complete a task unless it is accepted or started, "
						"or has not been completed back to the stakeholder." ); 
			if ( task.state == RTS_COMPLETED ) {
				try {
					GetLastAssigned( task );
				}
				catch ( exception& e ) {
					return e.what();
				}
			}
			break;
		case RLF_REOPENED:
			if (!task.IsStakeHolder( requestor ) )
				return string( "You must be the stakeholder of a task to reopen it." );
			if (task.state == RTS_OPEN )
				return string( "You cannot reopen an open task." );
			break;
		case RLF_CANCELED:
			if (!task.IsStakeHolder( requestor ) )
				return string( "You must be the stakeholder of a task to cancel it." );
			if ( task.state >= RTS_COMPLETED )
				return string( "Task must be less the complete to cancel - do you want to close it?" );
			break;
		case RLF_CLOSED:
			if (!task.IsStakeHolder( requestor ) )
				return string( "You must be the stakeholder of a task to close it." );
			if ( !task.IsAssigned( requestor ) && task.state != RTS_COMPLETED )
				return string( "You cannot close a task that is neither assigned to you or completed. Maybe you want to cancel it?" ); 
			break;
		default:
			return string( "Unknown action." );
	}

	return string();
}

int Ripple::GetLastAssigned( const RippleTask& task ) {
	vector<int> history;

	GetLinearizedAssignmentHistory( task, history );

	vector<int>::iterator task_it = std::find( history.begin(), history.end(), task.assigned );

	if ( task_it == history.begin()  )
		throw RippleException( "There isn't a previous owner of this task." );

	return *( task_it - 1 );
}

void Ripple::GetLinearizedAssignmentHistory( const RippleTask& task, vector<int>& history  ) {
	history.clear();
	history.push_back( task.stakeholder );

	soci::session sql( pool );
	rowset<row> rs = (sql.prepare 
			<< "SELECT DISTINCT user_id FROM logs "
			<< "WHERE task_id=:task_id AND user_id <> :user_id ORDER BY created_date",
			use( task.task_id ), use( task.stakeholder ) );

	for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it) {	
		history.push_back( (*it).get<int>( 0 ) );		
	}

	//If currently assigned user isn't in the history, add to the end.
	if ( std::find( history.begin(), history.end(), task.assigned ) == history.end() )
		history.push_back( task.assigned );
}

void Ripple::InsertTask( RippleTask& task, soci::session* sql = NULL ) {

	bool own = false;
	if ( sql == NULL )  {
		own = true;
		sql = new soci::session( pool );
	}
	try {
		*sql << "INSERT INTO tasks VALUES ("
			<< ":task_id, :stakeholder, :assigned, :start_date,"
			<<" :due_date, :state, :parent_task)",
			use( static_cast<const RippleTask&>( task ) );

		*sql << "SELECT last_insert_rowid()", into( task.task_id );
	}
	catch ( ... ) {
		if ( own )
			delete sql;
		throw;
	}
	if ( own )
		delete sql;



}

void Ripple::InsertLog( RippleLog& log, soci::session* sql = NULL ) {

	if ( log.description.length() > 4096 )
		throw RippleException( "log entry must not exceed 4k characters" );

	bool own = false;
	if ( sql == NULL )  {
		own = true;
		sql = new soci::session( pool );
	}
	try {
		*sql << "INSERT INTO logs VALUES("
			<< ":log_id, :flavor, :description, :user_id, :task_id,"
			<< ":created_date)",
			use( static_cast<const RippleLog&>( log ) );

		*sql << "SELECT last_insert_rowid()", into( log.log_id );
	}
	catch ( ... ) {
		if ( own )
			delete sql;
		throw;
	}
	if ( own )
		delete sql;
}

void Ripple::UpdateTask( const RippleTask& task, RippleLog& log, int previously_assigned ) {

	soci::session sql( pool );
	transaction tr(sql);
	sql << "UPDATE tasks SET "
		<< "stakeholder=:stakeholder, assigned=:assigned, start_date=:start_date, "
		<< "due_date=:due_date, state=:state, parent_task=:parent_task "
		<< "WHERE task_id=:task_id",
		use( task );

	InsertLog( log, &sql );
	tr.commit();

	stringstream task_data, user_id;
	task_data << task;
	user_id << task.stakeholder;
	rc->SubmitEvent( user_id.str(), task_data.str() );
	if ( task.assigned != task.stakeholder ) {
		user_id.str( "" );
		user_id << task.assigned;
		rc->SubmitEvent( user_id.str(), task_data.str() );
	}
	else if ( previously_assigned != -1 && task.assigned != previously_assigned ){
		user_id.str( "" );
		user_id << previously_assigned;
		rc->SubmitEvent( user_id.str(), task_data.str() );
	}
}

