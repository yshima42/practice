#include "Cluster.hpp"

void Cluster::run()
{
	std::string dot[3] = {".  ", ".. ", "..."};
	int n = 0;

	while (1)
	{
		fd_set reading_set;
		fd_set writing_set;
		struct timeval timeout;
		int ret = 0;

		while (ret == 0)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			FD_ZERO(&writing_set);
			for (std::vector<int>::iterator it = _ready.begin(); it != _ready.end(); it++)
				FD_SET(*it, &writing_set);
			
			std::cout << "\rWaiting on a connection" << dot[n++] << std::flush;
			if (n == 3)
				n = 0;

			ret = select(_max_fd + 1, &reading_set, &writing_set, NULL, &timeout);
		}
		
		if (ret > 0)
		{
			for (std::vector<int>::iterator it = _ready.begin(); ret && it != _ready.end(); it++)
			{
				if (FD_ISSET(*it, &writing_set))
				{
					long ret = _sockets[*it]->second(*it);

					if (ret == 0)
						_ready.erace(it);
					else if (ret == -1)
					{
						FD_CLR(*it, &reading_set);
						_sockets.erase(*it);
						_ready.erase(it);
					}
					ret = 0;
					break;
				}
			}
			if (ret)
				std::cout << "\rReceived a connection!!" << std::flush;

			for (std::map
}
