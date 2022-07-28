#include "Cluster.hpp"
#include "Configuration.hpp"
#include "Webserv.hpp"
#include "ft_signal.hpp"

int main(int ac, char *av[]) {
	Configuration config;
	char *path;

	if (ac != 2) {
		std::cerr << "no no no..." << std::endl;
		return 0;
	}
	try {
		path = av[1];
		config = Configuration(path);

		Cluster C(config);
		C.loop();
	}
	catch(std::exception &e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
