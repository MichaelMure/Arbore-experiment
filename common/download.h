#ifndef DOWNLOAD_H
#define DOWNLOAD_H
#include <curl/curl.h>

class Download
{
	CURL* curl;
public:
	Download();
	~Download();

	// Return true if the download was successful
	bool Get(const char* url, const char* save_as);
};

extern Download download;
#endif

