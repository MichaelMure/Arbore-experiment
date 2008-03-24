#ifndef PF_EXCEPTION_H
#define PF_EXCEPTION_H
#include <exception>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

class StrException : public std::exception
{
	std::string err;
public:
	StrException(std::string _err="") : err(_err) {}
	~StrException() throw() {}
	void SetString(const std::string& _err) { err = _err; }
	const std::string GetString() const { return err; }
};

#endif // PF_EXCEPTION_H
