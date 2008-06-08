/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include <cstdlib>
#include <curl/curl.h>
#include "download.h"
#include "pf_log.h"

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
	pf_log[W_INFO] << "Downloading " << url;
	if(fd == NULL)
	{
		pf_log[W_ERR] << "Download failed.";
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
		pf_log[W_ERR] << "Received http error " << http_error;
	}

	if(r != CURLE_OK)
		pf_log[W_ERR] << "Download failed, curl error : " << r;

	return (r == CURLE_OK);
}
