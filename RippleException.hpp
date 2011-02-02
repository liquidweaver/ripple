#ifndef RIPPLEEXCEPTION_H
#define RIPPLEEXCEPTION_H
#include <exception>
#include <string>

class RippleException : public exception {
	public:
		RippleException( string what )
			: m_what( what ) { }

		virtual ~RippleException() throw() {}

		virtual const char* what() const throw() {
			return m_what.c_str();
		}
	
	protected:
		string m_what;
};

#endif //RIPPLEEXCEPTION_H
