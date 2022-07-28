#ifndef PRA2_HPP
# define PRA2_HPP

#include <iostream>

class pra2
{
    public:
		pra2(std::string str);
        virtual ~pra2();
        pra2(pra2 const &other);
        pra2 &operator=(pra2 const &other);

    private:
};

#endif
