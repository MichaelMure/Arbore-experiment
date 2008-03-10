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
 * $Id$
 */

#include <iostream>
#include <syslog.h>
#include "tools.h"
#include "log.h"

Log log;

static struct
{
	uint32_t flag;
	int level;
	const char* s;
} all_flags[] =
{
	{ W_DEBUG,      LOG_DEBUG,   "DEBUG"      },
	{ W_PARSE,      LOG_DEBUG,   "PARSE"      },
	{ W_DESYNCH,    LOG_WARNING, "DESYNCH"    },
	{ W_WARNING,    LOG_WARNING, "WARNING"    },
	{ W_BIGWARNING, LOG_WARNING, "BIGWARNING" },
	{ W_ERR,        LOG_ERR,     "ERR"        },
	{ W_CONNEC,     LOG_NOTICE,  "CONNEC"     },
	{ W_INFO,       LOG_NOTICE,  "INFO"       }
};

Log::flux::~flux()
{
	int i;

	if(!(flag & log.LoggedFlags()))
		return;

	for(i = (sizeof all_flags / sizeof *all_flags) - 1; i >= 0 && !(flag & all_flags[i].flag); --i)
		;

	if(i < 0)
		syslog(LOG_WARNING, "[SYSLOG] (%X) Unable to find how to log this message: %s", flag, str.c_str());
	else
	{
		if(all_flags[i].level == LOG_ERR || all_flags[i].level == LOG_WARNING)
			syslog(all_flags[i].level, "[%s] %s", all_flags[i].s, str.c_str());
		(all_flags[i].level == LOG_ERR ? std::cerr : std::cout) << "[" << all_flags[i].s << "] " << str << std::endl;
	}
}

Log::Log()
			: logged_flags(DEFAULT_LOGGED_FLAGS)
{
	openlog("P2Pfs", LOG_CONS, LOG_DAEMON);
}

Log::~Log()
{
	closelog();
}

void Log::SetLoggedFlags(std::string s)
{
	std::string token;

	logged_flags = 0;

	while((token = stringtok(s, " ")).empty() == false)
	{
		int i;
		if(token == "ALL")
		{
			logged_flags = (uint32_t) -1;
			break;
		}

		for(i = (sizeof all_flags / sizeof *all_flags) - 1; i >= 0 && (token != all_flags[i].s); --i)
			;

		if(i >= 0)
			logged_flags |= all_flags[i].flag;
	}
}
