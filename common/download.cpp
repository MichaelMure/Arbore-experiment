#include <cstdlib>
#include <curl/curl.h>
#include "download.h"
#include "log.h"

Download download;

Download::Download() : curl(curl_easy_init())
{
}

Download::~Download()
{
	curl_easy_cleanup(curl);
}

size_t download_callback(void* buf, size_t size, size_t nmemb, void* fd)
{
	fwrite(buf, size, nmemb, (FILE*)fd);
	return nmemb;
}

bool Download::Get(const char* url, const char* save_as)
{
	FILE* fd = fopen( save_as, "w");
	log[W_INFO] << "Downloading " << url;
	if(fd == NULL)
	{
		log[W_ERR] << "Download failed.";
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_FILE, fd);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Peerfuse");
//	curl_easy_setopt(curl, CURLOPT_PROXY, "address");
//	curl_easy_setopt(curl, CURLOPT_PROXYPORT, 3128);
	
	
	CURLcode r = curl_easy_perform(curl);
	fclose(fd);
	
	if(r == CURLE_HTTP_RETURNED_ERROR)
	{
		long http_error = 0;
		curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE , &http_error);
		log[W_ERR] << "Received http error " << http_error;
	}

	if(r != CURLE_OK)
		log[W_ERR] << "Download failed, curl error : " << r;

	return (r == CURLE_OK);
}
