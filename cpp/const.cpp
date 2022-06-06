#include <iostream>
using namespace std;

class myclass {
    private:
        int a;
    public:
        myclass();
        ~myclass();
        void show();
};

myclass::myclass()
{
    cout << "constructor" << endl;
    a = 10;
}

myclass::~myclass()
{
    cout << "destructor" << endl;
}

void myclass::show()
{
    cout << a << endl;
}

int main()
{
    myclass ob;
    ob.show();
    return 0;
}
