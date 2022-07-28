#include "webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Cluster.hpp"

int main(int ac, char *av[])
{
	if (ac != 2)
	{
		std::cerr << "Invalid number of args." << std::endl;
		return 1;
	}

	Cluster cluster;
	try {
		cluster.config(av[1]);
		if (cluster.setup() == -1)
			return (1);
		cluster.run();
		cluster.clean();
	}
	chatch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
