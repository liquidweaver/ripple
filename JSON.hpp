#ifndef JSON_HPP
#define JSON_HPP
#include <string>

class JSON {
public:
		static std::string escape( std::string raw_string ) {
			const string quote( "\"" );
			const string escaped_quote( "\\\"" );

			size_t pos = raw_string.find(quote);
			while( pos != string::npos ) 
				raw_string.replace( pos, quote.size(), escaped_quote );

			return raw_string;
		}
	
		static std::string rfc3339( time_t timestamp ) {
			struct tm * timeinfo;
			char buffer [80];

			timeinfo = gmtime( &timestamp );

			strftime( buffer, 80, "%Y-%m-%dT%H:%M:%SZ", timeinfo);

			return string( buffer );
		}
};

#endif //JSON_HPP
