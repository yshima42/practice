#include <vector>
#include <iostream>
#include <array>

int main()
{
	std::array<int, 10> a ;

    // どちらも0番目の要素に1を代入
    a.at(0) = 1 ;
    a[0] = 1 ;

    // どちらも0番目の要素を標準出力
    std::cout << a.at(0) ;
    std::cout << a[0] ;
}
