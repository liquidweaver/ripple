#ifndef RIPPLEEXCEPTION_H
#define RIPPLEEXCEPTION_H
#include <exception>
#include <string>

class RippleException : public std::runtime_error {
	public:
		RippleException( std::string what )
			: std::runtime_error( what ) { }
};

#endif //RIPPLEEXCEPTION_H
