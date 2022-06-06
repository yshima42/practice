#include <iostream>
#include <map>
using namespace std;

int main()
{
	map<char, int> m;
	int i;

	for (i = 0; i < 26; i++)
		m.insert(pair<char, int>('A' + i, 65 + i));

	map<char, int>::iterator p;

	for (p = m.begin(); p != m.end(); p++) {
		cout << p->first << "has ";
		cout << p->second << endl;
		}

		return 0;
		}
