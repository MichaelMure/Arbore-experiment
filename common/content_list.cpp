#include <string>
#include "content_list.h"

ContentList content_list;

FileContent& ContentList::GetFile(std::string path)
{
	iterator it = find(path);
	if(it == end())
		insert(make_pair(path, FileContent(path, 0)));
	it = find(path);
	return it->second;
}

void ContentList::Loop()
{
	sleep(1);
}
