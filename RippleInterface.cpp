#include "RippleInterface.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h> //Some helper functions for the c glue

RippleInterface* RippleInterface::instance = NULL;
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
const char * RippleInterface::authorize_url = "/authorize";
const char * RippleInterface::login_url = "/login.html";
const char * RippleInterface::ajax_reply_start =
  "HTTP/1.1 200 OK\r\n"
  "Cache: no-cache\r\n"
  "Content-Type: application/x-javascript\r\n"
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
  ctx = mg_start(&event_handler, options);
  assert(ctx != NULL);

}

void * RippleInterface::event_handler(enum mg_event event,
                                             struct mg_connection *conn,
                                             const struct mg_request_info *request_info) {
  void *processed = reinterpret_cast<void*>(const_cast<char*>( "yes" ) );
  RippleInterface* inst = RippleInterface::instance;

  if (event == MG_NEW_REQUEST) {
    if (!request_info->is_ssl) {
      inst->redirect_to_ssl(conn, request_info);
    } else if (!inst->is_authorized(conn, request_info)) {
      inst->redirect_to_login(conn, request_info);
    } else if (strcmp(request_info->uri, authorize_url) == 0) {
      inst->authorize(conn, request_info);
    } else if (strcmp(request_info->uri, "/ajax/get_data") == 0) {
      //Call ajax dataa requestor
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
  if (!strcmp(request_info->uri, login_url) ||
      !strcmp(request_info->uri, authorize_url)) {
    return 1;
  }

  pthread_rwlock_rdlock(&rwlock);
  if ((session = get_session(conn)) != NULL) {
    generate_session_id( valid_id, session->random, session->user_id );
    if (strcmp(valid_id, session->session_id) == 0) {
      session->expire = time(0) + SESSION_TTL;
      authorized = 1;
    }
  }
  pthread_rwlock_unlock(&rwlock);

  return authorized;
}

void RippleInterface::redirect_to_ssl(struct mg_connection *conn,
                            const struct mg_request_info *request_info) {
  const char *p, *host = mg_get_header(conn, "Host");
  if (host != NULL && (p = strchr(host, ':')) != NULL) {
    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
              "Location: https://%.*s:8082/%s:8082\r\n\r\n",
              p - host, host, request_info->uri);
  } else {
    mg_printf(conn, "%s", "HTTP/1.1 500 Error\r\n\r\nHost: header is not set");
  }
}

void RippleInterface::redirect_to_login(struct mg_connection *conn,
                              const struct mg_request_info *request_info) {
  mg_printf(conn, "HTTP/1.1 302 Found\r\n"
      "Set-Cookie: original_url=%s\r\n"
      "Location: %s\r\n\r\n",
      request_info->uri, login_url);
}

// A handler for the /authorize endpoint.
// Login page form sends user name and password to this endpoint.
void RippleInterface::authorize(struct mg_connection *conn,
                      const struct mg_request_info *request_info) {
  char user_as_string[MAX_USER_LEN], password[MAX_USER_LEN];
  struct session *session;

  // Fetch user name and password.
  get_qsvar(request_info, "user", user_as_string, sizeof(user_as_string));
  get_qsvar(request_info, "password", password, sizeof(password));
  int user = atoi( user_as_string );

  if (check_password(user, password) && (session = new_session()) != NULL) {
    // Authentication success:
    //   1. create new session
    //   2. set session ID token in the cookie
    //   3. remove original_url from the cookie - not needed anymore
    //   4. redirect client back to the original URL
    //
    // The most secure way is to stay HTTPS all the time. However, just to
    // show the technique, we redirect to HTTP after the successful
    // authentication. The danger of doing this is that session cookie can
    // be stolen and an attacker may impersonate the user.
    // Secure application must use HTTPS all the time.
    snprintf(session->random, sizeof(session->random), "%d", rand());
    generate_session_id(session->session_id, session->random, session->user_id );
    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
        "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
        "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
        "Location: /\r\n\r\n",
        session->session_id);
  } else {
    // Authentication failure, redirect to login.
    redirect_to_login(conn, request_info);
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
  return i == MAX_SESSIONS ? NULL : &sessions[i];
}

// Generate session ID. buf must be 33 bytes in size.
// Note that it is easy to steal session cookies by sniffing traffic.
// This is why all communication must be SSL-ed.
void RippleInterface::generate_session_id(char *buf, const char *random,
                                const int user_id ) {
  char user_id_str[24];
  snprintf( user_id_str, sizeof( user_id_str ), "%d", user_id );
  mg_md5(buf, random, user_id_str, NULL);
}

void RippleInterface::get_qsvar(const struct mg_request_info *request_info,
                      const char *name, char *dst, size_t dst_len) {
  const char *qs = request_info->query_string;
  mg_get_var(qs, strlen(qs == NULL ? "" : qs), name, dst, dst_len);
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
  return i == MAX_SESSIONS ? NULL : &sessions[i];
}

bool RippleInterface::check_password( int user, const char* pass ) {
  //STUB
  return true;
}

// vim: ts=2 sw=2 ai et
