#pragma once
#ifndef FDSETMANAGER_HPP
#define FDSETMANAGER_HPP

#include "Webserv.hpp"

class FdSetManager
{
public:
	FdSetManager();
	
	void add(int fd, fd_set& set);
	void remove(int fd, fd_set& set);
	void clear(fd_set& set);
	bool isSet(int fd, const fd_set& set) const;
	void updateMaxFd(int fd);
	int getMaxFd() const;
	
private:
	int _max_fd;
};

#endif

