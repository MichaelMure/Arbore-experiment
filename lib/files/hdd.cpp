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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stack>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/mutex.h"
#include "util/pf_log.h"
#include "util/session_config.h"
#include "hdd.h"

Hdd hdd;

Hdd::Hdd() : Mutex(RECURSIVE_MUTEX)
{
}

Hdd::~Hdd()
{
}

//void Hdd::DirEntry& GetTree()
			//{
			//	return tree;
			//}

void Hdd::BuildTree(DirEntry* cache_dir, std::string _root)
{
	BlockLockMutex lock(this);
	root = _root;

	if(root.empty())
		throw HddAccessFailure(root);

	// Remove trailing / if needed
	if(root[root.size() -1] == '/')
		root = root.substr(0, root.size() -1);

	std::stack<std::string> path;
	std::string fullpath = root;

	DIR* d = opendir(fullpath.c_str());
	if(!d)
		throw HddAccessFailure(root);

	while(1)
	{

		struct dirent* dir;
		FileEntry* f;
		while((dir = readdir(d)))
		{
			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;

			std::string f_path = fullpath + "/" + std::string(dir->d_name);

			struct stat stats;
			if(lstat(f_path.c_str(), &stats))
				throw HddAccessFailure(f_path);

			pf_stat file_stats;
			file_stats.atime = stats.st_atime;
			file_stats.mtime = stats.st_mtime;
			file_stats.ctime = stats.st_ctime;

			// Add file
			if(stats.st_mode & S_IFDIR)
				f = new DirEntry(std::string(dir->d_name), file_stats, cache_dir);
			else
				f = new FileEntry(std::string(dir->d_name), file_stats, cache_dir);

			f->LoadAttr();

			pf_log[W_INFO] << f->GetFullName() << " loaded.";

			cache_dir->AddFile(f);
			if(stats.st_mode & S_IFDIR)
				break;
		}

		if(!dir)
		{
			// End of dir, go up
			closedir(d);
			if(!path.size())
				break;
			// Remove the last dir name from fullpath:
			std::string::size_type slash = fullpath.rfind("/");
			fullpath = fullpath.substr(0, slash);
			d = opendir(fullpath.c_str());
			if(!d)
				throw HddAccessFailure(root);

			// Go to the right position
			while((dir = readdir(d)) && strcmp(dir->d_name, path.top().c_str()))
				continue;

			if(!d)
						  // If harddrive cache changed during reading it
				throw HddAccessFailure(root);
			path.pop();
			cache_dir = cache_dir->GetParent();
		}
		else
		{
			// Go down this dir
			path.push(std::string(dir->d_name));
			fullpath = fullpath + "/" + std::string(dir->d_name);
			closedir(d);

			d = opendir(fullpath.c_str());
			if(!d)
						  // If harddrive cache changed during reading it
				throw HddAccessFailure(root);
			cache_dir = static_cast<DirEntry*>(f);
		}
	}
}

void Hdd::MkFile(FileEntry* f)
{
	BlockLockMutex lock(this);
	std::string path = root + f->GetFullName();

	mode_t mode = f->GetAttr().mode;
	mode &= S_IFREG | S_IFDIR;
	mode |= S_IRUSR | S_IWUSR | (mode & S_IFDIR ? S_IXUSR : 0);

	if(mode & S_IFDIR)
	{
		/* Try to open it to know if it exists */
		DIR* d = opendir(path.c_str());
		if(d)
		{
			//TODO: We may need to change perms
			closedir(d);
			return;
		}

		int r = mkdir(path.c_str(), mode);
		if(r)
		{
			/* Try to create parent directory. */
			if(!f->GetParent())
				throw HddWriteFailure(path);
			try
			{
				MkFile(f->GetParent());
			}
			catch(HddWriteFailure &e)
			{
				throw HddWriteFailure(path);
			}
			r = mkdir(path.c_str(), mode);
			if(r)
			{
				pf_log[W_ERR] << "mkdir failed: " << strerror(errno);
				throw HddWriteFailure(path);
			}
		}
		pf_log[W_INFO] << "mkdir on " << path;
	}
	else
	{
		int hdd_fd = open(path.c_str(), O_RDONLY);
		if(hdd_fd != -1)
		{
			// TODO: change perms
			close(hdd_fd);
			return;
		}

		int fd = creat(path.c_str(), mode);
		if(fd == -1)
		{
			/* Try to create parent directory. */
			if(!f->GetParent())
				throw HddWriteFailure(path);
			try
			{
				MkFile(f->GetParent());
			}
			catch(HddWriteFailure &e)
			{
				throw HddWriteFailure(path);
			}

			fd = creat(path.c_str(), mode);
			if(fd == -1)
				throw HddWriteFailure(path);
		}
		close(fd);
		pf_log[W_INFO] << "creat on " << path;
	}
}

void Hdd::RmFile(FileEntry* f)
{
	BlockLockMutex lock(this);
	std::string path = root + f->GetFullName();
	if(f->GetAttr().mode & S_IFDIR)
	{
		int r = rmdir(path.c_str());
		if(r)
			throw HddWriteFailure(path);
		pf_log[W_INFO] << "rmdir on " << path;
	}
	else
	{
		int fd = unlink(path.c_str());
		if(fd == -1)
			throw HddWriteFailure(path);
		pf_log[W_INFO] << "unlink on " << path;
	}
}

int Hdd::GetFd(std::string path)
{
	BlockLockMutex lock(this);
	std::string full_path = root + path;
	int f = open(full_path.c_str(), O_CREAT|O_RDWR);
	if(f == -1)
		pf_log[W_ERR] << "Unable to load file \"" << full_path << "\" from harddisk :" << strerror(errno);
	return f;
}
