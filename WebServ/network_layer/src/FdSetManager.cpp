#include "FdSetManager.hpp"

FdSetManager::FdSetManager() : _max_fd(0) {}

/**
 * Adds file descriptor to fd_set for select() monitoring
 * 
 * Example: Monitor new client fd=10 for reading
 * Before: _read_set = {5, 6}
 * After:  _read_set = {5, 6, 10}
 * _max_fd = 10
 * 
 * select() will now notify when fd=10 has data to read
 */
void FdSetManager::add(int fd, fd_set& set)
{
	FD_SET(fd, &set);
	updateMaxFd(fd);
}

/**
 * Removes file descriptor from fd_set
 * 
 * Example: Stop monitoring fd=10 for writing
 * Before: _write_set = {10, 15}
 * After:  _write_set = {15}
 */
void FdSetManager::remove(int fd, fd_set& set)
{
	FD_CLR(fd, &set);
}

/**
 * Clears all file descriptors from set
 * 
 * Example: Initialize empty set
 * _read_set = {} (no fds monitored)
 */
void FdSetManager::clear(fd_set& set)
{
	FD_ZERO(&set);
}

/**
 * Checks if fd is in set (after select() returns)
 * 
 * Example: Check if fd=10 has data ready
 * select() returned, read_cpy modified
 * isSet(10, read_cpy) → true = data available
 */
bool FdSetManager::isSet(int fd, const fd_set& set) const
{
	return FD_ISSET(fd, &set);
}

/**
 * Updates maximum fd value (required for select())
 * 
 * Example: Adding fd=10
 * _max_fd = 6 → 10
 * select() needs max_fd+1 = 11
 */
void FdSetManager::updateMaxFd(int fd)
{
	if (fd > _max_fd)
		_max_fd = fd;
}

/**
 * Returns highest fd number for select()
 * 
 * Example: _max_fd = 10
 * Call select(11, ...) - must be max_fd + 1
 */
int FdSetManager::getMaxFd() const
{
	return _max_fd;
}

