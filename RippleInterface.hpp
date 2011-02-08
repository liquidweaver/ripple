#ifndef RIPPLEINTERFACE_HPP
#define RIPPLEINTERFACE_HPP

#include <stdlib.h>
#include "mongoose/mongoose.h"
#include <memory>
#include <stdexcept>
#include "Ripple.hpp"
//60 mins
#define SESSION_TTL 60 * 60 
#define MAX_EMAIL_LEN 40
#define MAX_SESSIONS 250

// Describes web session.
struct session {
  char session_id[33];// Session ID, must be unique
  char random[20];    // Random data used for extra user validation
  int user_id;        // Authenticated user
  time_t expire;      // Expiration timestamp, UTC
};

class RippleInterface {
	public:
		static RippleInterface* Instance( Ripple* ripple, const char** options ); 

	private:
		RippleInterface( Ripple* ripple, const char** options );
		static void * event_handler(enum mg_event event, struct mg_connection *conn, 
				const struct mg_request_info *request_info);
		int is_authorized(const struct mg_connection *conn, const struct mg_request_info* request_info );
		void logout( const struct mg_connection* conn, const struct mg_request_info* request_info );
		void redirect_to_login(struct mg_connection *conn, const struct mg_request_info *request_info); 
		// A handler for the /authorize endpoint.
		// Login page form sends user name and password to this endpoint.
		void authorize(struct mg_connection *conn, const struct mg_request_info *request_info); 
		struct session* get_session(const struct mg_connection *conn);
		void generate_session_id(char *buf, const char *random, const int user_id );
		void get_qsvar(const struct mg_request_info *request_info, const char *name, char *dst, size_t dst_len);
		struct session *new_session(void);
		bool check_password( const char* email, const char* pass );

		static RippleInterface* instance;
		// Protects sessions and access to the Ripple class
		static pthread_rwlock_t rwlock;
		Ripple* ripple;
		session sessions[MAX_SESSIONS];  // Current sessions
		//constants
		static const char* authorize_url;
		static const char* logout_url;
		static const char* login_url;
		static const char* ajax_reply_start;
};

#endif //RIPPLEINTERFACE_HPP
