#include "Configuration.hpp"

Configuration::Configuration()
{
}

Configuration::Configuration(std::string file)
{
	_parse_config(file);
	_complete_config();
}

#define BUFFER_SIZE 256

std::string read_file(std::string file)
{
	char buffer[BUFFER_SIZE + 1] = {0};
	int fd;
	int i;
	int res;
	std::string result;

	fd = open(file.c_str(), O_RDONLY);
	if (fd < 0)
		throw ParsingExpectation(0, "The file does not exists.");
	while ((res = read(fd, buffer, BUFFER_SIZE)) > 0)
	{
		result += buffer;
		i = 0;
		while(i < BUFFER_SIZE)
			buffer[i++] = 0;
	}
	if (res < 0)
		throw ParsingExpectation(0, "error while reading.");
	close(fd);
	return(result);
}

size_t count_lines(std::string source)
{
	size_t i;
	size_t lines;

	i = 0;
	lines = 1;
	while(source[i])
	{
		if(source[i++] == '\n')
			++lines;
	}
	return (lines);
}

std::string get_line(std::string source, size_t n)
{
	size_t i;
	size_t j;
	size_t line_count;

	if(n >= count_lines(source))
		return (std::string());
	line_count = 0;
	i = 0;
	while(line_count < n)
	{
		if(source[i++] == '\n')
			++line_count;
	}
	while(std::isspace(source[i]) && source[i] != '\n')
		++i;
	j = 0;
	while(source[i + j] && source[i + j] != '\n')
		++j;
	while(j > 0 && std::isspace(source[i + j - 1]))
		--j;
	return (std::string(source, i, j));
}

std::vector< std::string > split_white_space(std::string str)
{
	std::vector< std::string > res;
	size_t i;
	size_t j;

	i = 0;
	j = 0;
	while(str[i])
	{
		if(std::isspace(str[i]))
		{
			if(i == j)
				++j;
			else
			{
				res.push_back(std::string(str, j, i - j));
				j = i + 1;
			}
		}
		++i;
	}
	if(i != j)
		res.push_back(std::string(str, j, i - j));
	return (res);
}

bool is_skippable(std::string source, size_t line)
{
	std::string l;

	l = get_line(source, line);
	return (split_white_space(l).size() == 0 || l.size() == 0 || l[0] == '#');
}

void Configuration::_parse_server(std::string source, size_t line_start, size_t line_end) {

}

void Configuration::_parse_config(std::string file) {
	std::string file_content;
	size_t i;
	size_t size;
	std::vector < std::string > line;

	i = 0;
	file_content = read_file(file);
	size = count_lines(file_content);
	while (i < size) {
		if (!is_skippable(file_content, i)) {
			line = split_white_space(get_line(file_content, i));
			if (line.size() > 0 && line[0] == "server") {
				_parse_server(file_content, i, get_closing_bracket(file_content, i));
				i = get_closing_bracket(file_content, i);
			}
			else
				throw ParsingException(i, "Unexpected token " + line[0]);
		}
		++i;
	}
	_validate_config();
}


	
void Configuration::_complete_config() {
}

Configuration::~Configuration()
{
}

Configuration::Configuration(Configuration const &other)
{
    *this = other;
}

Configuration &Configuration::operator=(Configuration const &other)
{
    if (this != &other)
    {
    }
    return *this;
}
