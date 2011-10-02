/*
 * Copyright(C) 2011 Michael Muré <batolettre@gmail.com>
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
 *
 */

#include <unistd.h>
#include <assert.h>
#include "file_chunk_desc.h"

FileChunkDesc::FileChunkDesc()
 : offset(0), size(0)
{
}

FileChunkDesc::FileChunkDesc(off_t _offset, size_t _size)
 : offset(_offset), size(_size)
{
}

bool FileChunkDesc::operator==(const FileChunkDesc &other)
{
	return GetOffset() == other.GetOffset() && GetSize() == other.GetSize();
}

bool FileChunkDesc::operator<(const FileChunkDesc &other)
{
	return GetOffset() < other.GetOffset() || (GetOffset() == other.GetOffset() && GetSize() < other.GetSize());
}

off_t FileChunkDesc::GetOffset() const
{
	return offset;
}

size_t FileChunkDesc::GetSize() const
{
	return size;
}

off_t FileChunkDesc::GetEndOffset() const
{
	return GetOffset() + GetSize();
}

bool FileChunkDesc::Overlaps(const FileChunkDesc& other) const
{
	return  (GetOffset() <= other.GetOffset() && GetEndOffset() > other.GetOffset())
		|| (other.GetOffset() <= GetOffset() && other.GetEndOffset() > GetOffset());
}

FileChunkDesc FileChunkDesc::GetCommonPartDesc(const FileChunkDesc& other)
{
	if(!Overlaps(other))
		return FileChunkDesc();
	off_t begin_off = MAX(GetOffset(), other.GetOffset());
	off_t end_off = MIN(GetEndOffset(), other.GetEndOffset());
	assert(begin_off < end_off);

	return FileChunkDesc(begin_off, (size_t)(end_off - begin_off));
}

bool FileChunkDesc::Contains(const FileChunkDesc& other) const
{
	return GetOffset() <= other.GetOffset() && GetEndOffset() >= other.GetEndOffset();
}
