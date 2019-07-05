#include <iostream>

using namespace std;

int main()
{
    int sum = 0;
    for(auto i = 1; i <= 100; i++){
        sum+=i;
    }

    cout << "result: " << sum << endl;
}
