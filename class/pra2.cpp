#include "pra2.hpp"

pra2::pra2(std::string str)
{
	std::cout << str << std::endl;
}

pra2::~pra2()
{
}

pra2::pra2(pra2 const &other)
{
    *this = other;
}

pra2 &pra2::operator=(pra2 const &other)
{
    if (this != &other)
    {
    }
    return *this;
}
