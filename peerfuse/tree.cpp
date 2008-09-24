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

#include <stack>

#include "files/dir_entry.h"
#include "file_content.h"
#include "content_list.h"
#include "hdd.h"
#include "tree.h"
#include "environment.h"

Tree tree;

Tree::Tree()
	: Mutex(RECURSIVE_MUTEX)
{
	this->tree = NULL;
}

Tree::~Tree()
{
	if(this->tree)
		delete this->tree;
}

FileEntry* Tree::Path2File(std::string path, unsigned int flags, std::string* filename)
{
	BlockLockMutex lock(this);
	DirEntry* current_dir = this->tree;

	std::string name;

	while((name = stringtok(path, "/")).empty() == false)
	{
		FileEntry* child_file = current_dir->GetFile(name);
		if(!child_file || child_file->IsRemoved())
		{
			if(path.empty())
			{
				/* we are in last dir, but this file doesn't exist */
				if(child_file && (flags & (RESTORE_REMOVED_FILE|GET_REMOVED_FILE)))
				{
					if(flags & RESTORE_REMOVED_FILE)
						child_file->ClearRemoved();
					return child_file;
				}
				else if(filename)
				{
					*filename = name;
					return current_dir;
				}
				else
					return NULL;
			}

			/* we aren't in last dir, so the path isn't found. */
			if(flags & CREATE_UNKNOWN_DIRS)
			{
				if(child_file && !dynamic_cast<DirEntry*>(child_file))
				{
					current_dir->RemFile(child_file);
					child_file = NULL;
				}

				if(!child_file)
				{
					pf_stat stat;
					stat.uid = 0;
					stat.gid = 0;
					child_file = new DirEntry(name, stat, current_dir);
					current_dir->AddFile(child_file);
				}
				if(flags & RESTORE_REMOVED_FILE)
					child_file->ClearRemoved();
			}
			else if(!(flags & GET_REMOVED_FILE) || !child_file)
				return NULL;
		}

		if(!(current_dir = dynamic_cast<DirEntry*>(child_file)))
		{
			/* This isn't a directory. */
			if(path.empty())
			{
				/* We are on last dir, so it is the requested file. */
				return child_file;
			}
			/* it isn't a file in path, so the path isn't found. */

			return NULL;
		}
	}

	return current_dir;
}

void Tree::Load(std::string hd_path)
{
	BlockLockMutex lock(this);
	this->tree = new DirEntry("", pf_stat(), NULL);
	pf_stat s = this->tree->GetAttr();
	s.mode = S_IFDIR | S_IRWXU;
	this->tree->SetAttr(s);
	try
	{
		hdd.BuildTree(GetTree(), hd_path);
	}
	catch(...)
	{
		throw;
	}
}

void Tree::ChOwn(std::string path, uid_t uid, gid_t gid)
{
	BlockLockMutex lock(this);

	FileEntry* file = Path2File(path);
	if(!file)
		throw NoSuchFileOrDir();

	pf_stat stat = file->GetAttr();
	stat.uid = uid;
	stat.gid = gid;
	stat.meta_mtime = time(NULL);
	SetAttr(path, stat);
}

void Tree::ChMod(std::string path, mode_t mode)
{
	BlockLockMutex lock(this);

	FileEntry* file = Path2File(path);
	if(!file)
		throw NoSuchFileOrDir();

	pf_stat stat = file->GetAttr();
	stat.mode = mode;
	stat.meta_mtime = time(NULL);
	SetAttr(path, stat);
}

pf_stat Tree::GetAttr(std::string path)
{
	BlockLockMutex lock(this);

	FileEntry* file = Path2File(path);
	if(!file)
		throw NoSuchFileOrDir();

	return file->GetAttr();
}

void Tree::Write(std::string path, const char* buf, size_t size, off_t off)
{
	FileContent& file = content_list.GetFile(path);
	FileChunk chunk(buf, off, size);
	file.SetChunk(chunk);

	/* No need to lock cache, we don't touch its members */
	pf_stat stat = GetAttr(path);
	time_t now = time(NULL);
	stat.meta_mtime = now;
	stat.mtime = now;
	stat.ctime = now;
	KeyList keylist;
	keylist.insert(environment.my_key.Get());
	if(off + (off_t)size > (off_t)stat.size)
	{
		stat.size = (size_t)off + size;
		SetAttr(path, stat, keylist);
	}

	//content_list.RefreshPeersRef(path);
}

int Tree::Read(std::string path, char* buf, size_t size, off_t off)
{
	off_t file_size = GetAttr(path).size;

	if(off >= file_size)
	{
		pf_log[W_WARNING] << "Fuse trying to read out of file";
		return 0;
	}

	FileContent& file = content_list.GetFile(path);

	/* Limit the read to the size of the file */
	size_t to_read = (size_t) ((off + (off_t)size > file_size) ? file_size - off : size);

	if(!to_read)
	{
		pf_log[W_WARNING] << "Fuse trying to read out of file";
		return 0;
	}

	FileChunkDesc chunk_to_read(off, to_read);
	FileContent::chunk_availability chunk_state;
	while((chunk_state = file.HaveChunk(chunk_to_read)) == FileContent::CHUNK_NOT_READY)
		usleep(100000);			  /* 0.1 sec */

	if(chunk_state == FileContent::CHUNK_UNAVAILABLE)
		throw FileUnavailable();

	FileChunk chunk = file.GetChunk(chunk_to_read);

	if(!chunk.GetData())			  /* shouldn't happen */
	{
		pf_log[W_WARNING] << "FileContent returned an empty chunk??";
		return 0;
	}

	memcpy(buf, chunk.GetData(), to_read);

	/* TODO CHECK THIS CONVERSION. FUSE CAN ONLY RETURN AN INTEGER, SO IT IS NEEDED. */
	return (int)to_read;
}

int Tree::Truncate(std::string path, off_t offset)
{
	pf_log[W_DEBUG] << "Truncating \"" << path << "\" at " << offset;

	pf_stat stat = GetAttr(path);
	stat.size = (size_t)offset;
	stat.ctime = time(NULL);
	stat.mtime = stat.ctime;
	stat.meta_mtime = stat.ctime;

	KeyList keylist;
	keylist.insert(environment.my_key.Get());

	SetAttr(path, stat, keylist);

	FileContent& file = content_list.GetFile(path);
	file.Truncate(offset);

	return 0;
}

bool Tree::FileExists(std::string path)
{
	BlockLockMutex lock(this);
	return Path2File(path) != NULL;
}


FileList Tree::GetAllFiles()
{
	/* Stack used to store states of each directories */
	std::stack<std::pair<FileMap::const_iterator, FileMap::const_iterator> > stack;
	FileList list;

	BlockLockMutex lock(this);
	DirEntry* current_dir = GetTree();

	/* GetFiles() returns a reference, so we can use it directly */
	FileMap::const_iterator it = current_dir->GetFiles().begin();
	FileMap::const_iterator end = current_dir->GetFiles().end();

	list.insert(GetTree());

	while(current_dir)
	{
		for(; it != end; ++it)
		{
			DirEntry* dir = dynamic_cast<DirEntry*>(it->second);

			list.insert(it->second);

			if(dir)
			{
				current_dir = dir;
				break;
			}
		}
		/* End of dir, we go back on top folder */
		if(it == end)
		{
			current_dir = current_dir->GetParent();
			if(current_dir)
			{
				/* Restore state of parent dir */
				std::pair<FileMap::const_iterator, FileMap::const_iterator> iterators = stack.top();
				it = iterators.first;
				end = iterators.second;
				stack.pop();
			}
		}
		/* We enter in a subdir */
		else
		{
			/* Store current state in stack */
			stack.push(std::pair<FileMap::const_iterator, FileMap::const_iterator>(++it, end));

			/* And change iterators to next dir */
			it = current_dir->GetFiles().begin();
			end = current_dir->GetFiles().end();
		}

	}

	return list;
}

void Tree::MkFile(std::string path, pf_stat stat, KeyList sharers, Key sender)
{
	std::string filename;
	FileEntry* file = 0;
	BlockLockMutex lock(this);

	try
	{
		file = Path2File(path, CREATE_UNKNOWN_DIRS|GET_REMOVED_FILE, &filename);
		DirEntry* dir = dynamic_cast<DirEntry*>(file);

		if(!file)
			throw NoSuchFileOrDir();

		if(filename.empty() || !dir)
			throw FileAlreadyExists();

		if(!sender)
		{
			sharers.insert(environment.my_key.Get());
			stat.uid = environment.my_key.Get();
			stat.gid = environment.my_key.Get();
		}

		if(stat.mode & S_IFDIR)
		{
			stat.mode |= S_IRWXU;
			file = new DirEntry(filename, stat, dir);
		}
		else
			file = new FileEntry(filename, stat, dir);

		file->SetAttr(stat); // hack to force stat to saved
		file->SetSharers(sharers);

		dir->AddFile(file);

		pf_log[W_DEBUG] << "Created " << (stat.mode & S_IFDIR ? "dir " : "file ") << filename << " in " << path << ". There are " << dir->GetSize() << " files and directories";

		/* Write file on cache */
		hdd.MkFile(file);

		/* Add file on FileDistribution */
		filedist.AddFile(file, sender);

	}
	catch(Tree::FileAlreadyExists &e)
	{
		/* If it me who wants to create this file, we go out
		 * to let fuse returns an error.
		 */
		if(sender == 0)
		{
			if(file->IsRemoved())
				file->ClearRemoved();
			else
				throw;
		}

		pf_log[W_DEBUG] << "File already exists... Update it.";

		_set_attr(file, stat, sender, sharers);
	}
}

void Tree::RmFile(std::string path)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	// If it's a dir that isn't empty -> return error
	if (f->GetAttr().mode & S_IFDIR)
	{
		DirEntry* d = static_cast<DirEntry*>(f);
		if(d->CountExistantFiles() != 0)
			throw DirNotEmpty();
	}

	if(!f->GetParent())
		throw NoPermission();

	pf_log[W_DEBUG] << "Removed " << path;

	pf_stat stat = f->GetAttr();
	stat.pf_mode |= FilePermissions::S_PF_REMOVED;
	stat.meta_mtime = time(NULL);
	stat.mtime = stat.meta_mtime;

	_set_attr(f, stat, 0, KeyList());

	FileContent& file = content_list.GetFile(path);
	file.Truncate(0);
	content_list.RemoveFile(f->GetFullName());

}

void Tree::SetAttr(std::string path, pf_stat stat, KeyList sharers, Key sender, bool keep_newest, bool erase_on_modification)
{
	BlockLockMutex lock(this);
	std::string filename;
	FileEntry* file = Path2File(path, GET_REMOVED_FILE, &filename);
	DirEntry* dir = dynamic_cast<DirEntry*>(file);

	if(!file || (filename.empty() == false && dir))
		throw NoSuchFileOrDir();

	_set_attr(file, stat, sender, sharers);
}

void Tree::_set_attr(FileEntry* file, pf_stat stat, Key sender, KeyList sharers, bool keep_newest, bool erase_on_modification)
{
	BlockLockMutex cache_lock(this);
	BlockLockMutex peer_lock(&peers_list);

	if(sender)
	{
		/* This file already exists, but do not panic! We take modifications only if
		 * this file is more recent than mine.
		 */
		time_t dist_ts = stat.meta_mtime;

		if(file->GetAttr().meta_mtime > dist_ts)
		{
			/* My file is more recent than peer's, so I send it a mkfile
			 * to correct this.
			 */
			pf_log[W_DEBUG] << "My file is more recent than peer's, so I correct him";
			Packet pckt = filedist.CreateMkFilePacket(file);
			pckt.SetDstID(sender);
			peers_list.SendMsg(sender, pckt);
			return;
		}
		else if(file->GetAttr() == stat)
		{
			pf_log[W_DEBUG] << "Same timestamp...";
			/* TODO Same timestamp, what can we do?... */
			return;			  /* same version, go out */
		}
		else if(erase_on_modification && file->GetAttr().mtime != stat.mtime)
		{
			FileContent& file_content = content_list.GetFile(file->GetFullName());
			file_content.Truncate(0);
		}
		else if(stat.size < file->GetAttr().size)
		{
			FileContent& file_content = content_list.GetFile(file->GetFullName());
			file_content.Truncate(stat.size);
		}
	}
	else
	{
		time_t now = time(NULL);

		/* If local meta_mtime == now, to prevent flood we delay NET_MKFILE message
		 * to the next second
		 */
		if(file->GetAttr().meta_mtime == now)
		{
			std::string filename = file->GetFullName();
			if(delayed_mkfile_send.find(filename) == delayed_mkfile_send.end())
				delayed_mkfile_send[filename] = KeyList();
		}
		else
			stat.meta_mtime = now;
	}

	pf_log[W_DEBUG] << "Updating file.";

	if(sharers.empty() == false)
	{
		/* If distant file has not the same content that mine, we get distant
		 * sharers list.
		 * In other case, we only merge list.
		 */
		if(file->GetAttr().mtime != stat.mtime)
		{
			pf_log[W_DEBUG] << file->GetAttr().mtime << " != " << stat.mtime << ": we update sharers";
			file->SetSharers(sharers);
		}
		else
		{
			pf_log[W_DEBUG] << file->GetAttr().mtime << " == " << stat.mtime << ": we merge sharers";
			KeyList keylist = file->GetSharers();
			keylist.insert(sharers.begin(), sharers.end());
			file->SetSharers(keylist);
		}
	}

	file->SetAttr(stat);
	if(file->IsRemoved())
	{
		FileContent& f = content_list.GetFile(file->GetFullName());
		f.Truncate(0);
		file->SetSharers(KeyList());
	}

	/* Update file */
	hdd.UpdateFile(file);
	filedist.AddFile(file, sender);
}

void Tree::AddSharer(std::string path, Key sharer)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	KeyList idl = f->GetSharers();
	if(std::find(idl.begin(), idl.end(), sharer) != idl.end())
		return;

	idl.insert(sharer);
	pf_stat stat = f->GetAttr();
	stat.meta_mtime = time(NULL);

	_set_attr(f, stat, 0, idl);

}

void Tree::SendMkFile(std::string filename)
{
	BlockLockMutex lock(this);

	FileEntry* f = Path2File(filename, GET_REMOVED_FILE);

	if(!f)
		throw NoSuchFileOrDir();

	std::map<std::string, KeyList>::iterator it;
	if((it = delayed_mkfile_send.find(filename)) == delayed_mkfile_send.end())
		return;

	pf_log[W_DEBUG] << "Send delayed MKFILE:";
	Packet p = filedist.CreateMkFilePacket(f);
	peers_list.SendMsg(it->second, p);

	delayed_mkfile_send.erase(it);

}

void Tree::RenameFile(std::string path, std::string new_path, Key sender)
{
	BlockLockMutex lock(this);

	/* TODO implement this */
}

void Tree::SendDirFiles(std::string path, Key to)
{
	BlockLockMutex lock(this);
	DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path));

	if(!dir)
		throw NoSuchFileOrDir();

	filedist.SendDirFiles(dir, to);
}

#ifndef PF_SERVER_MODE
void Tree::SetReadyForList(std::string path)
{
	BlockLockMutex lock(this);
	dir_incoming[path] = FINISHED;
}

bool Tree::IsReadyForList(std::string path)
{
	BlockLockMutex lock(this);

	std::map<std::string, incoming_states_t>::iterator incoming = dir_incoming.find(path);
	if(incoming == dir_incoming.end() || incoming->second == FINISHED)
		return true;
	else
		return false;
}

void Tree::FillReadDir(const char* _path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	std::string path = _path;
	bool wait_for_content = false;

	/* Open a block only to use BlockLockMutex. */
	do
	{
		/* Do NOT lock mutex outside of this block! It might freeze Tree because
		 * of the loop
		 */
		BlockLockMutex lock(this);

		DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path));

		if(!dir)
			throw NoSuchFileOrDir();

		path = dir->GetFullName();

		/* If I'm not responsible of this directory, send a request to a responsible
		 * to get content.
		 */
		if(!filedist.IsResponsible(environment.my_key.Get(), dir))
		{
			BlockLockMutex lock(&peers_list);
			std::set<Key> peers = filedist.GetRespPeers(dir);

			if(peers.empty())
				pf_log[W_WARNING] << "Tree::FillReadDir: there isn't any responsible of " << dir->GetFullName() << " ??";
			else
			{
				std::map<std::string, incoming_states_t>::iterator incoming = dir_incoming.find(path);
				if(incoming == dir_incoming.end() || incoming->second != GETTING)
				{
					Key resp = *peers.begin();

					Packet pckt = Packet(NET_LS_DIR, environment.my_key.Get(), resp);
					pckt.SetArg(NET_LS_DIR_PATH, path);
					peers_list.SendMsg(resp, pckt);

					dir_incoming[path] = GETTING;
				}

				wait_for_content = true;
			}

		}

	}
	while(0);

	if(wait_for_content)
	{
		while(!IsReadyForList(path))
			usleep(10000);		  /* 0.01 sec */

	}

	BlockLockMutex lock(this);
	DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path));

	if(!dir)
		throw NoSuchFileOrDir();

	FileMap files = dir->GetFiles();
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		if(it->second->IsRemoved())
			continue;

		struct stat st;
		memset(&st, 0, sizeof st);
		/*st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;*/

		if(filler(buf, it->second->GetName().c_str(), &st, 0))
			break;
	}
}
#endif						  /* PF_SERVER_MODE */

void Tree::UpdateRespFiles()
{
	BlockLockMutex lock(this);
	filedist.UpdateRespFiles();
}

void Tree::RequestFileRefs(std::string filename)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(filename);

	if(!f)
		throw NoSuchFileOrDir();

	filedist.RequestFileRefs(f);
}
