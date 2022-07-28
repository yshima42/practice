#include "pra.hpp"

pra::pra()
{
}

pra::~pra()
{
}

void pra::use_pra2()
{
	std::string hello = "hello";
	pra2_ = new pra2(hello);
}

pra::pra(pra const &other)
{
    *this = other;
}

pra &pra::operator=(pra const &other)
{
    if (this != &other)
    {
    }
    return *this;
}
