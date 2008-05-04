#ifndef CRL_H
#define CRL_H
#include <string>
#include <openssl/x509.h>
#include "pf_exception.h"

class Crl
{
	std::string filename;
	X509_CRL* crl;
	bool disabled;
public:
	Crl();
	~Crl();

	class BadCRL : public StrException
	{
	public:
		BadCRL(std::string err) : StrException(err) {}
	};

	void Load(std::string _path);

	X509_CRL* GetSSL() { return crl; }
	void Disable() { disabled = true; }
	bool GetDisabled() const { return disabled; }
};

extern Crl crl;

#endif /* CRL_H */
