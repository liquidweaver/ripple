#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h> //Some helper functions for the c glue
#include <iostream>
#include "RippleInterface.hpp"
#include "Ripple.hpp"
#include "mongoose/mongoose.h"
#include "JSON.hpp"

RippleInterface* RippleInterface::instance = NULL;
pthread_rwlock_t RippleInterface::rwlock = PTHREAD_RWLOCK_INITIALIZER;
const char* RippleInterface::authorize_url = "/authorize";
const char* RippleInterface::logout_url = "/logout";
const char* RippleInterface::login_url = "/login.html";
const char* RippleInterface::signup_url = "/signup.html";
const char* RippleInterface::signup_ajax_url = "/signup";
const char* RippleInterface::ajax_reply_start =
  "HTTP/1.1 200 OK\r\n"
  "Cache: no-cache\r\n"
  "Content-Type: application/json\r\n"
  "\r\n";

const char* RippleInterface::http500 = 
  "HTTP/1.1 500 Internal Server Error\r\n\r\n";

std::ostream& operator<<(std::ostream& out, const RippleLog& log ) {
	out << '{'
		<< '"' << "log_id" << "\":" << log.log_id << ','
		<< '"' << "flavor" << "\":" << log.flavor << ','
		<< '"' << "body" <<  "\":\"" << JSON::escape( log.body ) << '"' << ','
		<< '"' << "subject" <<  "\":\"" << JSON::escape( log.Subject() ) << '"' << ','
		<< '"' << "user_id" << "\":"  << log.user_id << ','
		<< '"' << "task_id" << "\":" << log.task_id << ','
		<< '"' << "created_date" << "\":\"" << JSON::rfc3339( log.created_date ) << '"'
		<< '}';
}

std::ostream& operator<<(std::ostream& out, const RippleTask& task ) {

	out << '{'
		<< '"' << "task_id" << "\":" << task.task_id << ','
		<< '"' << "stakeholder" << "\":" << task.stakeholder << ','
		<< '"' << "start_date" << "\":\"" << JSON::rfc3339( task.start_date ) << "\","
		<< '"' << "due_date" << "\":\"" << JSON::rfc3339( task.due_date ) << "\","
		<< '"' << "state" << "\":"  << task.state << ','
		<< '"' << "parent_task" << "\":" << task.parent_task;
		
	try { 
		RippleUser stakeholder;
		Ripple::Instance()->GetUser( task.stakeholder, stakeholder );
		out << ",\"" << "stakeholder_name\":\"" << JSON::escape( stakeholder.name ) << "\"";
	}
	catch ( ... ) { }

	try { 
		RippleUser assigned;
		Ripple::Instance()->GetUser( task.assigned, assigned );
		out << ",\"" << "assigned_name\":\"" << JSON::escape( assigned.name ) << "\"";
	}
	catch ( ... ) { }



	vector<int> log_ids;

	//Good thing Ripple is a singleton...
	Ripple::Instance()->GetLogsForTask( task, log_ids );


	if ( log_ids.size() > 0 ) {
		out << ",\"logs\":[";

		for( vector<int>::const_iterator log_id = log_ids.begin(); log_id != log_ids.end(); ++log_id ) {
			RippleLog log;
			Ripple::Instance()->GetLog( *log_id, log );
			out << log;
			if ( log_id + 1 != log_ids.end() )
				out << ',';
		}

		out << ']';
	}
	out << '}';
}
  

RippleInterface* RippleInterface::Instance( Ripple* ripple, const char** options ) {
  if ( instance == NULL )
    instance = new RippleInterface( ripple, options );

  return instance;
}

void RippleInterface::Release() {
	if ( instance != NULL )
		delete instance;
	
	instance = NULL;

}

RippleInterface::RippleInterface( Ripple* ripple, const char** options )
  : ripple( ripple ) {

  struct mg_context *ctx;

  // Initialize random number generator. It will be used later on for
  // the session identifier creation.
  srand((unsigned) time(0));

  // Setup and start Mongoose
  ctx = mg_start(&event_handler, NULL,  options);
  assert(ctx != NULL);

}

void * RippleInterface::event_handler( mg_event event, mg_connection* conn,
                                             const mg_request_info* request_info) {
  void *processed = reinterpret_cast<void*>(const_cast<char*>( "yes" ) );
  RippleInterface* inst = RippleInterface::instance;
  string post_data;
  if ( !strcmp( request_info->request_method, "POST" ) ) {
    char buf[8192];
    int len = mg_read( conn, buf, sizeof( buf ) );
    if ( len > 0 ) 
      post_data = string( buf, len ); 
  }



  if (event == MG_NEW_REQUEST) {
    if (!inst->is_authorized(conn, request_info)) {
      inst->redirect( conn, request_info,login_url );
    } else if (strcmp(request_info->uri, authorize_url ) == 0) {
      inst->authorize(conn, request_info, post_data );
    } else if (strcmp(request_info->uri, signup_ajax_url ) == 0) {
      inst->signup(conn, request_info, post_data );
    } else if (strcmp(request_info->uri, logout_url ) == 0) {
      inst->logout(conn, request_info);
      inst->redirect( conn, request_info, login_url );
    } else if (strcmp(request_info->uri, "/ajax/query") == 0) {
      int user_id = -1;
      pthread_rwlock_rdlock(&rwlock);
      try {
        user_id = inst->get_session(conn)->user_id;
      }
      catch ( exception&e ) { }
      pthread_rwlock_unlock(&rwlock);
      if ( user_id != -1 ) {
        try {
          RippleUser user;
          inst->ripple->GetUser( user_id, user );
          inst->query( conn, request_info, post_data, user );
        }
        catch( exception& e ) {
          mg_printf( conn, http500 );
        }
      }
    } else {
      // No suitable handler found, mark as not processed. Mongoose will
      // try to serve the request.
      processed = NULL;
    }
  } else {
    processed = NULL;
  }

  return processed;
}

int RippleInterface::is_authorized(const struct mg_connection *conn,
                         const struct mg_request_info *request_info) {
  struct session *session;
  char valid_id[33];
  int authorized = 0;

  // Always authorize accesses to login page and to authorize URI
  if (!strcmp(request_info->uri, login_url ) ||
      !strcmp(request_info->uri, authorize_url ) ||
      !strcmp(request_info->uri, signup_url ) ||
      !strcmp(request_info->uri, signup_ajax_url ) ) {
    return 1;
  }
  else if ( strlen( request_info->uri ) > 4 &&
            !strcmp( &request_info->uri[ strlen( request_info->uri ) - 4 ], ".css" ) ) {
    return 1;
  }
  else if ( strlen( request_info->uri ) > 4 &&
            !strcmp( &request_info->uri[ strlen( request_info->uri ) - 4 ], ".png" ) ) {
    return 1;
  }
  else if ( strlen( request_info->uri ) > 4 &&
            !strcmp( &request_info->uri[ strlen( request_info->uri ) - 4 ], ".jpg" ) ) {
    return 1;
  }
  else if ( strlen( request_info->uri ) > 3 &&
            !strcmp( &request_info->uri[ strlen( request_info->uri ) - 3 ], ".js" ) ) {
    return 1;
  }

  pthread_rwlock_rdlock(&rwlock);
  try {
    session = get_session(conn);
    generate_session_id( valid_id, session->random, session->user_id );
    if (strcmp(valid_id, session->session_id) == 0) {
      session->expire = time(0) + SESSION_TTL;
      authorized = 1;
    }
  }
  catch ( ... ) {
  }
  pthread_rwlock_unlock(&rwlock);

  return authorized;
}

void RippleInterface::logout( const struct mg_connection * conn, const struct mg_request_info* request_info ) {

  struct session* session;
  int user_id = -1;
  
  pthread_rwlock_rdlock(&rwlock);
  try {
    session = get_session(conn);
    session->expire = 0;
    user_id = session->user_id;
  }
  catch ( ... ) { }
  pthread_rwlock_unlock(&rwlock);

  if ( user_id != -1 ) {
    try {
      RippleUser ru;
      ripple->GetUser( user_id, ru );
      cout << "logout - " << ru.name << endl;
    }
    catch ( ... ) { }
    
  }
}

void RippleInterface::redirect( struct mg_connection *conn,
                                const struct mg_request_info *request_info,
                                const char* target ) {
  mg_printf(conn, "HTTP/1.1 302 Found\r\n"
      "Set-Cookie: original_url=%s\r\n"
      "Location: %s\r\n\r\n",
      request_info->uri, target );
}

// A handler for the /authorize endpoint.
// Login page form sends user name and password to this endpoint.
void RippleInterface::authorize( mg_connection *conn,
                                 const mg_request_info *request_info,
                                 const string& post_data ) {
  session *session;

  // Fetch user name and password.
  string email = get_post_var( post_data, "email" );
  string password = get_post_var( post_data, "password" );

  try {
    if ( email == "" )
      throw runtime_error( "You must supply an email address." );
    if ( password == "" )
      throw runtime_error( "You must supply a password." );
    
    RippleUser ru = ripple->GetUserFromEmailAndPassword(email, password) ;
    cout << "login - " << ru.name << endl;
    session = new_session(); 
    session->user_id = ru.user_id;
    // Authentication success:
    //   1. create new session
    //   2. set session ID token in the cookie
    //   3. remove original_url from the cookie - not needed anymore
    //   4. redirect client back to the original URL
    snprintf(session->random, sizeof(session->random), "%d", rand());
    generate_session_id(session->session_id, session->random, session->user_id );
    mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                    "Cache: no-cache\r\n"
                    "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
                    "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
                    "Content-Type: application/json\r\n\r\n{ \"error_msg\": \"\" }",
                    session->session_id);
  }
  catch ( RippleException& e ) {
    // Authentication failure, redirect to login.
    mg_printf( conn, "%s{ \"error_msg\": \"%s\" }", ajax_reply_start, e.what() );
  }
  catch ( exception& e ) { //Unknown error occured
    //TODO: Communicate that something crazy is going on
    mg_printf( conn, "%s{ \"error_msg\": \"%s\" }", ajax_reply_start, e.what() );
  }

}

void RippleInterface::signup( struct mg_connection *conn,
                              const struct mg_request_info *request_info,
                              const string& post_data ) {

  string name = get_post_var( post_data, "name" );
  string email = get_post_var( post_data, "email" );
  string password = get_post_var( post_data, "password" );
  cout << "signup - name: " << name << ", email: " << email << ", password: " << password << endl;
  string error_msg;
  try {
    RippleUser ru( name, email );
    ru.password = password;

    ripple->InsertUser( ru );
  }
  catch( exception& e ) {
    error_msg = e.what();
  }

  mg_printf( conn, "%s{ \"error_msg\": \"%s\", \"name\":\"%s\" }", ajax_reply_start, error_msg.c_str(), name.c_str() );

}
void RippleInterface::query( struct mg_connection *conn,
                             const struct mg_request_info *request_info,
                             const string& post_data,
                             const RippleUser& user ) {
  string method = get_post_var( post_data, "method" );

  try {
    if ( method != "" ) {
	 	if ( method == "create_task" ) {
			string description = get_post_var( post_data, "description" );
			string start_date = get_post_var( post_data, "start_date" );
			string due_date = get_post_var( post_data, "due_date" );
			cout << "New task - start_date: " << start_date
					<< " due_date: " << due_date
					<< " description: " << description << endl;
			time_t t_start_date = start_date == "" ? -1 : atoi( start_date.c_str() );
			time_t t_due_date = due_date == "" ? -1 : atoi( due_date.c_str() );
			try {
				ripple->CreateTask( user, description, t_start_date, t_due_date ); 

				mg_printf( conn, "%s{ \"error_msg\":\"\" }", ajax_reply_start );
			}
			catch ( exception& e ) {
				mg_printf( conn, "%s{ \"error_msg\":\"%s\" }", ajax_reply_start, e.what() );
			}
		}
      if ( method == "get_assigned_tasks" ) {
        vector<int> task_ids;
        ripple->GetUsersAssignedTasks( user.user_id, task_ids, false );

        if ( task_ids.size() > 0 ) {
          ostringstream json;
          json << '[';
          for  ( vector<int>::const_iterator task_id = task_ids.begin(); task_id != task_ids.end(); ++task_id ) {
            RippleTask rt;
            ripple->GetTask( *task_id, rt );
            json << rt;  
				 if ( task_id + 1 != task_ids.end() )
					json << ',';
          }
          json << ']';

			 mg_printf( conn, "%s%s", ajax_reply_start, json.str().c_str() );
        }
        else {
          mg_printf( conn, "%s[]", ajax_reply_start );
         }
       
      }
    }
    else {
      throw runtime_error( "Invalid query." );
    }
  }
  catch ( exception& e ) {
    cerr << "RippleInterface::query(): " << e.what() << endl;
    mg_printf( conn, http500 );
  }

}

// Get session object for the connection. Caller must hold the lock.
struct session* RippleInterface::get_session(const struct mg_connection *conn) {
  int i;
  char session_id[33];
  time_t now = time(NULL);
  mg_get_cookie(conn, "session", session_id, sizeof(session_id));
  for (i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].expire != 0 &&
        sessions[i].expire > now &&
        strcmp(sessions[i].session_id, session_id) == 0) {
      break;
    }
  }
  if ( i == MAX_SESSIONS )
    throw RippleInterfaceException( "No session exists for that connection." );
  return &sessions[i];
}

// Generate session ID. buf must be 33 bytes in size.
// Note that it is easy to steal session cookies by sniffing traffic.
// This is why all communication must be SSL-ed.
void RippleInterface::generate_session_id(char *buf, const char *random, const int user_id ) {
  char user_id_str[24];
  snprintf( user_id_str, sizeof( user_id_str ), "%d", user_id );
  mg_md5(buf, random, user_id_str, NULL);
}

string RippleInterface::get_post_var( const string& post_data, const string& name ) {
  char buf[4096];
  mg_get_var( post_data.c_str(), post_data.length(), name.c_str(), buf, sizeof( buf ) );

  return string( buf );
}

struct session* RippleInterface::new_session(void) {
  int i;
  time_t now = time(NULL);
  pthread_rwlock_wrlock(&rwlock);
  for (i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].expire == 0 || sessions[i].expire < now) {
      sessions[i].expire = time(0) + SESSION_TTL;
      break;
    }
  }
  pthread_rwlock_unlock(&rwlock);
  if ( i == MAX_SESSIONS )
    throw RippleInterfaceException( "No more sessions available, sorry." );
  return &sessions[i];
}
