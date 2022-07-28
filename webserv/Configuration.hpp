#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


class Configuration
{
    public:
        Configuration();
		Configuration(std::string file);
        virtual ~Configuration();
        Configuration(Configuration const &other);
        Configuration &operator=(Configuration const &other);

    private:
		void _parse_config(std::string file);
		void _complete_config();
		void _parse_server(std::string source, size_t line_start, size_t line_end)


};

#endif
