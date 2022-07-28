#ifndef PRA_HPP
# define PRA_HPP

#include "pra2.hpp"

class pra
{
    public:
        pra();
        virtual ~pra();
        pra(pra const &other);
        pra &operator=(pra const &other);
		void use_pra2();

    private:
		pra2 *pra2_;
};

#endif
