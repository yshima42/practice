#pragma once

#include "webserv.hpp"

#include "Server.hpp"

class Cluster {
	public:
		Cluster();
		Cluster(const Cluster &src);
		~Cluster();

		Cluster &operator=(const Cluster & src);
		
		//int setup();
		void run();
		void clean();

	private:
		std::map<long, Server>	_servers;
		std::map<long, Server *>	_sockets;
		std::vector<int>	_ready;
		fd_set	_fd_set;
		unsigned int	_fd_size;
		long	_max_fd;
}

