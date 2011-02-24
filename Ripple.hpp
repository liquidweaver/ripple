#ifndef RIPPLE_H
#define RIPPLE_H
#include "RippleUser.hpp"
#include "RippleTask.hpp"
#include "RippleLog.hpp"
#include "RippleDefines.hpp"
#include "RippleException.hpp"
#include "restcomet/restcomet.h"
#include <soci/soci.h>

#ifndef EVENTS_PORT
	#define EVENTS_PORT 8081
#endif

class Ripple {
	public:

		static Ripple* Instance();
		static void Release();

		void InsertUser( RippleUser& ru );

		void GetUser( int user_id, RippleUser& user );
		void UpdateUser( const RippleUser& user );
		RippleUser GetUserFromEmailAndPassword( const string& email, const string& password );

		void DeleteUser( RippleUser& user );
		void DeleteUser( int user_id );

		RippleTask CreateTask( const RippleUser& ru, const string& subject_and_body, std::time_t start = -1, std::time_t due = -1 );
		void GetTask( int task_id, RippleTask& task );
		void GetUsersAssignedTasks( int user_id, vector<int>& tasks, bool includeAccepted = true );
		void GetLogsForTask( const RippleTask& task, vector<int>& logs );
		void GetLog( int log_id, RippleLog& log );

		void ReOpenTask( RippleTask& task, const RippleUser& requestor, const string& reason );
		void CancelTask( RippleTask& task, const RippleUser& requestor, const string& reason );
		void AcceptTask( RippleTask& task, const RippleUser& requestor, string reason = "" );
		void ForwardTask( RippleTask& task, const RippleUser& requestor, const RippleUser& target, string reason = "" );
		void RequestFeedback( RippleTask& task, const RippleUser& requestor, const string& reason );
		void DeclineTask( RippleTask& task, const RippleUser& requestor, const string& reason );
		void StartTask( RippleTask& task, const RippleUser& requestor, string reason = "" );
		void CompleteTask( RippleTask& task, const RippleUser& requestor, string reason = "" );
		void AddNoteToTask( const RippleTask& task, const RippleUser& requestor, const string& body );

		/** 
		 * @brief Generates a map of RIPPLE_TASK_FLAVORs to error strings
		 * 
		 * @param task The task in question
		 * @param requestor The user who wants to do an action
		 * @param actions A reference to a map that will be filled in with error strings.
		 * @note For each RIPPLE_TASK_FLAVOR that has an emptry string, the user can performa that action on the task.
		 * 	A non-empty string contains the reason why they cannot perform that action on the task.
		 */
		void GetPossibleActions( const RippleTask& task, const RippleUser& requestor, map<RIPPLE_LOG_FLAVOR, string>& actions );

		//Needs to access db functions to work
		friend std::ostream& operator<<(std::ostream& out, const RippleTask& task );
	private:
		static Ripple* instance;
		Ripple();
		Ripple( const Ripple& ripple ) { throw logic_error( "not allowed" ); }
		Ripple operator=( const Ripple& ripple ) { throw logic_error( "not allowed" ); }

		vector<RIPPLE_LOG_FLAVOR> KnownFlavors;

		string CheckAction( const RippleTask& task, const RippleUser& requestor, RIPPLE_LOG_FLAVOR flavor );

		/** 
		 * @brief Returns the previously assigned user of this task
		 * 
		 * @param task The task in question.
		 * 
		 * @return 
		 * @throw RippleException if no previous user. 
		 */
		int GetLastAssigned( const RippleTask& task );


		/** 
		 * @brief Returns a distint history of user_id's for a given task, based on the logs
		 * 
		 * @param task The task in question.
		 * @param history A reference to a vector<int> to store the history to. The first
		 * 			element is always the stakeholder.
		 */
		void GetLinearizedAssignmentHistory( const RippleTask& task, vector<int>& history  );

		void InsertTask( RippleTask& task );
		void InsertLog( RippleLog& log );
		void UpdateTask( const RippleTask& task, RippleLog& log );

		soci::session sql;
		rc::restcomet* rc;
};
#endif//RIPPLE_H
