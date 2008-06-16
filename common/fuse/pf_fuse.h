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
 * $Id$
 */

#ifndef P2PFS_FUSE
#define P2PFS_FUSE
#include <fuse.h>

int pf_chmod(const char* path, mode_t mode);
int pf_chown(const char *path, uid_t uid, gid_t gid);
int pf_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int pf_flush(const char *path, struct fuse_file_info *fi);
int pf_getattr(const char *path, struct stat *stbuf);
int pf_mkdir(const char *path, mode_t mode);
int pf_chmod(const char*, mode_t);
int pf_readdir(const char*, void*, int (*)(void*, const char*, const struct stat*, off_t), off_t, fuse_file_info*);
int pf_rename(const char* path, const char* new_path);
int pf_rmdir(const char *path);
int pf_unlink(const char *path);
int pf_utimens(const char *path, const struct timespec ts[2]);
int pf_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int pf_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int pf_truncate(const char* path, off_t offset);
void* fuse_init(struct fuse_conn_info* fuse_info);
void fuse_destroy(void*);

extern struct fuse_operations pf_oper;
#endif						  // P2PFS_FUSE
