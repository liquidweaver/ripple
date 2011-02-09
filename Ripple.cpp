#include "Ripple.hpp"
#include <algorithm> //std::find
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <iostream> //DEBUGGING

using namespace soci;

backend_factory const &backEnd = *soci::factory_sqlite3();
Ripple::Ripple() {
	sql.open( backEnd, "ripple.db"  );
	sql << "PRAGMA foreign_keys = ON";

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
}

void Ripple::InsertUser( RippleUser& ru ) {
	assert( ru.user_id == -1 );

	if ( ru.email == "UNSET" || ru.email == "" )
		throw RippleException( "Email must be specified" );
	if ( ru.name == "UNSET" || ru.name == "" )
		throw RippleException( "Name must be specified" );
	if ( ru.password == "UNSET" || ru.password == "" )
		throw RippleException( "Password must be specified" );
	

	sql << "INSERT INTO users VALUES (:user_id, :name, :email, :password)",
		 use( static_cast<const RippleUser&>( ru ) );

	sql << "SELECT last_insert_rowid()", into( ru.user_id );

	if ( !sql.got_data() )
		throw RippleException( "User could not be inserted." );
}

void Ripple::GetUser( int user_id, RippleUser& user ) {
	assert( user_id > 0 );
	RippleUser ru;
	indicator ind;
	sql << "SELECT * FROM users WHERE user_id=:user_id",
		 into( user, ind ), use( user_id );

	if ( !sql.got_data() ) 
		throw RippleException( "User not found." );
}

RippleUser Ripple::GetUserFromEmailAndPassword( const string& email, const string& password ) {
	RippleUser ru;
	indicator ind;
	sql << "SELECT * FROM users WHERE email=:email AND password=:password",
				use(ru, ind );

	if ( !sql.got_data() )
		throw RippleException( "Invalid email or password" );

	return ru;
}

void Ripple::DeleteUser( RippleUser& user ) {
	DeleteUser( user.user_id );
}

void Ripple::DeleteUser( int user_id ) {
	sql << "DELETE FROM users WHERE user_id=:user_id",
		use( user_id );

	int changes;
	sql << "SELECT changes()", into( changes );
	if ( changes == 0 )
		throw RippleException( "User not found." );
}

RippleTask Ripple::CreateTask( const RippleUser& ru, const string& subject_and_body, std::time_t start, std::time_t due ) {
	if ( subject_and_body == "" )
		throw RippleException( "You must enter a description when creating a task." );

	RippleTask rt( ru );
	rt.start_date = start;
	rt.due_date = due;
	InsertTask( rt );

	RippleLog rl( ru, rt, RLF_CREATED, subject_and_body );
	InsertLog( rl );

	return rt;
}

void Ripple::GetTask( int task_id, RippleTask& task ) {
	assert( task_id > 0 );

	sql << "SELECT * FROM tasks WHERE task_id=:task_id",
		into( task ), use( task_id );
	
	if ( !sql.got_data() )
		throw RippleException( "Task not found." );
}

void Ripple::GetLogsForTask( const RippleTask& task, vector<int>& logs ) {
	logs.clear();
	logs.resize( 500 );
	
	sql << "SELECT log_id FROM logs WHERE task_id=:task_id",
		into( logs ), use( task.task_id );
	
	if ( !sql.got_data() )
		throw RippleException( "No logs found. Does task exist?" );
}

RippleLog Ripple::GetLog( int log_id ) {
	RippleLog log;
	sql << "SELECT * FROM logs WHERE log_id=:log_id",
			into( log ), use( log_id );

	if ( !sql.got_data() )
		throw RippleException( "Log not found." );

	return log;
}

void Ripple::ReOpenTask( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_REOPENED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( reason == "" )
		throw RippleException( "You must enter a reason when reopening a task." );
	task.assigned = task.stakeholder; //Reset assigned to stakeholder
	task.state = RTS_OPEN;
	RippleLog log( requestor, task, RLF_REOPENED, reason );
	UpdateTask( task, log );
}

void Ripple::CancelTask( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_CANCELED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( reason == "" )
		throw RippleException( "You must enter a reason when canceling a task." );
	task.assigned = task.stakeholder; //Reset assigned to stakeholder
	task.state = RTS_CANCELED;
	RippleLog log( requestor, task, RLF_CANCELED, reason );
	UpdateTask( task, log );
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

	task.assigned = target.user_id;
	RippleLog log( requestor, task, RLF_FORWARDED, reason );
	UpdateTask( task, log );
}

void Ripple::RequestFeedback( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_FEEDBACK );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( reason == "" )
		throw RippleException( "You must enter a reason when requesting feedback for a task." );
	task.assigned = GetLastAssigned( task );
	RippleLog log( requestor, task, RLF_FEEDBACK, reason );
	UpdateTask( task, log );
}

void Ripple::DeclineTask( RippleTask& task, const RippleUser& requestor, const string& reason ) {
	string cannot = CheckAction( task, requestor, RLF_DECLINED );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( reason == "" )
		throw RippleException( "You must enter a reason when requesting declining a task." );
	task.assigned = GetLastAssigned( task );
	RippleLog log( requestor, task, RLF_DECLINED, reason );
	UpdateTask( task, log );
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

	try { //Pass this back to the previous if possible
		task.assigned = GetLastAssigned( task );
	}
	catch ( RippleException& e ) 
	{ }

	task.state = RTS_COMPLETED;
	RippleLog log( requestor, task, RLF_COMPLETED, reason );
	UpdateTask( task, log );
}

void Ripple::AddNoteToTask( const RippleTask& task, const RippleUser& requestor, const string& body ) {
	string cannot = CheckAction( task, requestor, RLF_NOTE );
	if ( cannot != "" )
		throw RippleException( cannot );

	if ( body == "" )
		throw RippleException( "Note must have some content." );
	RippleLog log( requestor, task, RLF_NOTE, body );
	InsertLog( log );
}

void Ripple::GetPossibleActions( const RippleTask& task, const RippleUser& requestor, map<RIPPLE_LOG_FLAVOR, string>& actions ) {
	actions.clear();

	for( vector<RIPPLE_LOG_FLAVOR>::const_iterator flavor = KnownFlavors.begin();
			flavor != KnownFlavors.end(); ++flavor ) {
		actions[*flavor] = CheckAction( task, requestor, *flavor );
	}
}

	string Ripple::CheckAction( const RippleTask& task, const RippleUser& requestor, RIPPLE_LOG_FLAVOR flavor ) {
		if ( task.state == RTS_CLOSED || task.state == RTS_CANCELED ) 
			return string( "Task is finalized; no actions can be performed." );
		if ( !task.IsAssigned( requestor ) && !task.IsStakeHolder( requestor ) )
			return string( "You are neither assigned or the stakeholder of this task." );

		switch( flavor ) {
			case RLF_CREATED: //Always allowed
				break;
			case RLF_NOTE:
				if ( !task.IsStakeHolder( requestor ) && ( task.state != RTS_ACCEPTED || task.state != RTS_STARTED ) )
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
				{ vector<int> history;
					GetLinearizedAssignmentHistory( task, history );
					if ( history.size() < 2 )
						return string( "There is noone to request feedback from." );
				}
				break;
			case RLF_DECLINED:
				if (!task.IsAssigned( requestor ) )
					return string( "You must be assigned to a task to decline it." );
				if ( task.state != RTS_OPEN )
					return string( "You cannot decline a task unless it is open." ); 
				{ vector<int> history;
					GetLinearizedAssignmentHistory( task, history );
					if ( history.size() < 2 )
						return string( "There is noone to decline to." );
				}
				break;
			case RLF_ACCEPTED:
				if (!task.IsAssigned( requestor ) )
					return string( "You must be assigned to a task to accept it." );
				if ( task.state != RTS_OPEN )
					return string( "You cannot accept a task unless it is open." ); 
				break;
			case RLF_STARTED:
				if (!task.IsAssigned( requestor ) )
					return string( "You must be assigned to a task to start it." );
				if ( task.state != RTS_OPEN && task.state != RTS_ACCEPTED )
					return string( "You cannot start a task unless it is open or accepted." ); 
				break;
			case RLF_COMPLETED:
				if (!task.IsAssigned( requestor ) )
					return string( "You must be assigned to a task to complete it." );
				if ( !(task.state == RTS_OPEN || task.state == RTS_ACCEPTED || 
							task.state == RTS_STARTED || task.state == RTS_COMPLETED ) )
					return string( "You cannot complete a task unless it is open, accepted, started, "
							"or has not been completed back to the stakeholder." ); 
				if ( task.state == RTS_COMPLETED ) {
					vector<int> history;
					GetLinearizedAssignmentHistory( task, history );
					if ( history.size() < 2 )
						return string( "There is noone to complete back to." );
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

void Ripple::InsertTask( RippleTask& task ) {

	sql << "INSERT INTO tasks VALUES ("
		<< ":task_id, :stakeholder, :assigned, :start_date,"
		<<" :due_date, :state, :parent_task)",
		use( static_cast<const RippleTask&>( task ) );

	sql << "SELECT last_insert_rowid()", into( task.task_id );
}

void Ripple::InsertLog( RippleLog& log ) {

	sql << "INSERT INTO logs VALUES("
		<< ":log_id, :flavor, :body, :user_id, :task_id,"
		<< ":created_date)",
		use( static_cast<const RippleLog&>( log ) );

	sql << "SELECT last_insert_rowid()", into( log.log_id );
}

void Ripple::UpdateTask( const RippleTask& task, RippleLog& log ) {

	sql << "UPDATE tasks SET "
		<< "stakeholder=:stakeholder, assigned=:assigned, start_date=:start_date, "
		<< "due_date=:due_date, state=:state, parent_task=:parent_task "
		<< "WHERE task_id=:task_id",
		use( task );

	InsertLog( log );
}
