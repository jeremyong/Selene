#include "luab.h"
#include <cassert>

int main() {
    Luab l;
    l.Load("../test/test.lua");
    l.Call("foo");

    int result = l.Call<int>("add", 5, 2);
    assert(result == 7);

    int sum, difference;
    std::tie(sum, difference) = l.Call<int, int>("sum_and_difference", 3, 1);
    assert(sum == 4 && difference == 2);

    int x;
    bool y;
    std::string z;
    std::tie(x, y, z) = l.Call<int, bool, std::string>("bar");
    assert(x == 4 && y == true && z == "hi");

    std::cout << "Call tests finished successfully." << std::endl;
}
