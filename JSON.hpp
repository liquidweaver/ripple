#ifndef JSON_HPP
#define JSON_HPP
#include <string>
#include <iostream>
#include <iomanip>

class JSON {
public:
		static std::string escape( std::string raw_string ) {
			const string need_to_escape( "\"\\" );
			stringstream quoted_string;

			for ( string::const_iterator achar = raw_string.begin();
					achar != raw_string.end(); ++achar ) {
				if ( *achar == '\"' || *achar == '\\' || iscntrl( *achar ) )
					quoted_string << "\\u00" << hex << setw(2) << setfill( '0' ) << static_cast<unsigned int>( *achar );
				else
					quoted_string << *achar;
			}

			return quoted_string.str();
		}
	
		static std::string rfc3339( time_t timestamp ) {
			if ( timestamp == -1 )
				return string();
			struct tm * timeinfo;
			char buffer [80];

			timeinfo = gmtime( &timestamp );

			strftime( buffer, 80, "%Y-%m-%dT%H:%M:%SZ", timeinfo);

			return string( buffer );
		}
};

#endif //JSON_HPP
