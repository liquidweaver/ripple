#include "RippleInterface.hpp"
#include "mongoose/mongoose.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h> //Some helper functions for the c glue
#include <iostream>

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

RippleInterface* RippleInterface::Instance( Ripple* ripple, const char** options ) {
  if ( instance == NULL )
    instance = new RippleInterface( ripple, options );

  return instance;
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
    } else if (strcmp(request_info->uri, "/ajax/get_data") == 0) {
      //Call ajax data requestor
    } else if (strcmp(request_info->uri, "/ajax/do_action") == 0) {
      //Call ajax action processor
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
  
  pthread_rwlock_rdlock(&rwlock);
  try {
    session = get_session(conn);
    session->expire = 0;
  }
  catch ( ... ) { }
  pthread_rwlock_unlock(&rwlock);
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
void RippleInterface::authorize( struct mg_connection *conn,
                      const struct mg_request_info *request_info,
                       const string& post_data ) {
  struct session *session;

  // Fetch user name and password.
  string email = get_post_var( post_data, "email" );
  string password = get_post_var( post_data, "password" );
  std::cout << "authorize - email: " << email << ", password: " <<  password << endl;

  try {
    RippleUser ru = ripple->GetUserFromEmailAndPassword(email, password) ;
    session = new_session(); 
    // Authentication success:
    //   1. create new session
    //   2. set session ID token in the cookie
    //   3. remove original_url from the cookie - not needed anymore
    //   4. redirect client back to the original URL
    snprintf(session->random, sizeof(session->random), "%d", rand());
    generate_session_id(session->session_id, session->random, session->user_id );
    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
        "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
        "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
        "Location: /\r\n\r\n",
        session->session_id);
  }
  catch ( RippleException& e ) {
    // Authentication failure, redirect to login.
    redirect( conn, request_info, login_url );
  }
  catch ( ... ) { //Unknown error occured
    //TODO: Communicate that something crazy is going on
    redirect( conn, request_info, login_url );
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

  mg_printf( conn, "%s{ \"error_msg\": \"%s\" }", ajax_reply_start, error_msg.c_str() );

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
// vim: ts=2 sw=2 ai et
