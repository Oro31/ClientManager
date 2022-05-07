#include "ClientManager.hpp"

/**** CONDESTRUCT ****/

ClientManager::ClientManager() : _fd(1), _read_ok(false),
	_send_ok(false), _read(""), _send("") {
}

ClientManager::ClientManager(int fd) : _fd(fd), _read_ok(false),
	_send_ok(false), _read(""), _send("") {
}

ClientManager::~ClientManager() {
}

/**** SETGET ****/

int	Socket::getFd() {
	return _fd;
}

bool	Socket::getSendOk() {
	return _send_ok;
}

bool	Socket::getReadOk() {
	return _read_ok;
}

std::string	Socket::getRead() {
	return _read;
}

std::string	Socket::getSend() {
	return _send;
}

void	Socket::setFd(int fd) {
	_fd = fd;
}

void	Socket::setSendOk(bool ok) {
	_send_ok = ok;
}

void	Socket::setReadOk(bool ok) {
	_read_ok = ok;
}

void	Socket::setRead(std::string str) {
	_read = str;
}

void	Socket::setSend(std::string str) {
	_send = str;
}

/**** USE ****/

void	Socket::appendRead(char *buf) {
	_read.append(buf);
}

bool	ClientManager::isReadOk() {
	HttpRequest	req(_read, _read.size());
	if (req.getBody().size() == req.getContentLength()) {
		_read_ok = true;
	}
	return _read_ok;
}

/**** SOCKET SERVER ****/

SocketServer::SocketServer(int port, int connections) : Socket() {
	int					opt = true;
	struct sockaddr_in	address;

	memset(&address, 0, sizeof(address));

	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		throw "INVALID SOCKET";
	}

	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
			sizeof(opt)) < 0) {
		throw "INVALID SOCKET";
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		throw "INVALID SOCKET";
	}
	if (listen(_server_fd, connections) < 0) {
		throw "INVALID SOCKET";
	}
}

int	SocketServer::getSocketUsed() const {
	return _sd;
}

void	SocketServer::setSocketUsed(int fd) {
	_sd = fd;
}

fd_set	SocketServer::getReadFds() const {
	return _readfds;
}

int	SocketServer::acceptSocket() {
	int	new_socket;
	int	addrlen = sizeof(_address);

	if ((new_socket = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t *)&addrlen)) < 0) {
		throw "INVALID SOCKET";
	}
	return new_socket;
}

void	SocketServer::selectSocket() {
	int									activity;
	std::map<int, std::string>::iterator			it;
	std::map<int, std::string>::const_iterator	ite = this->getClientSocket().end();

	FD_ZERO(&_readfds);
	FD_SET(_server_fd, &_readfds);
	_max_sd = _server_fd;

	for (it = this->getClientSocket().begin(); it != ite; it++) {
		_sd = it->first;
		if (_sd > 0)
			FD_SET(_sd, &_readfds);
		if (_sd > _max_sd)
			_max_sd = _sd;
	}

	activity = select(_max_sd + 1, &_readfds, NULL, NULL, NULL);

	if ((activity < 0) && (errno != EINTR))						//Don't use errno!!!
		throw "SELECT FAILED";
}

bool	SocketServer::ready(int fd, fd_set set) {
	if (FD_ISSET(fd, &set))
		return true;
	return false;
}

void	SocketServer::closeServerClean() {
	close(_sd);
	FD_CLR(_sd, &_readfds);
}

void	SocketServer::closeClient(int fd) {
	close(fd);
}
