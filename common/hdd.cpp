/*
 * Copyright(C) 2008 Laurent Defert
 *                   Romain Bignon <romain AT vaginus.org>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stack>
#include <string.h>
#include "hdd.h"
#include "log.h"
#include "session_config.h"

Hdd::Hdd()
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

			// Add file
			if(dir->d_type & DT_DIR)
				f = new DirEntry(std::string(dir->d_name), cache_dir);
			else
				f = new FileEntry(std::string(dir->d_name), cache_dir);

			std::string f_path = fullpath + "/" + std::string(dir->d_name);

			struct stat stats;
			if(lstat(f_path.c_str(), &stats))
				throw HddAccessFailure(f_path);

			f->stat.mode = stats.st_mode;
			f->stat.uid = stats.st_uid;
			f->stat.gid = stats.st_gid;
			f->stat.size = stats.st_size;
			f->stat.atime = stats.st_atime;
			f->stat.mtime = stats.st_mtime;
			f->stat.ctime = stats.st_ctime;
			if(!tree_cfg.Get(f_path, f->stat.meta_mtime))
			{
				f->stat.meta_mtime = f->stat.mtime;
				tree_cfg.Set(f_path, f->stat.meta_mtime);
			}

			cache_dir->AddFile(f);
			if(dir->d_type & DT_DIR)
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
	std::string path = root + f->GetFullName();
	if(f->stat.mode & S_IFDIR)
	{
		int r = mkdir(path.c_str(), f->stat.mode);
		if(r)
			throw HddWriteFailure(path);
		log[W_INFO] << "mkdir on " << path;
	}
	else
	{
		int fd = creat(path.c_str(), f->stat.mode);
		if(fd == -1)
			throw HddWriteFailure(path);
		close(fd);
		log[W_INFO] << "creat on " << path;
	}

	if(lchown(path.c_str(), f->stat.uid, f->stat.gid) != 0)
		throw HddWriteFailure(path);

	tree_cfg.Set(f->GetFullName(), f->stat.meta_mtime);
}

void Hdd::RmFile(FileEntry* f)
{
	std::string path = root + f->GetFullName();
	if(f->stat.mode & S_IFDIR)
	{
		int r = rmdir(path.c_str());
		if(r)
			throw HddWriteFailure(path);
		log[W_INFO] << "rmdir on " << path;
	}
	else
	{
		int fd = unlink(path.c_str());
		if(fd == -1)
			throw HddWriteFailure(path);
		log[W_INFO] << "unlink on " << path;
	}
}
