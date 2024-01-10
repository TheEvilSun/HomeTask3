#include "Allocators.h"
#include "List.h"
#include <map>
#include <iostream>

int main(int argc, char *argv[])
{
    std::map<int, int> map_default;
    std::map<int, int, std::less<int>, allocators::extensible::Allocator<std::pair<const int, int>, 5> > map_custom;
    containers::List<int> list_default;
    containers::List<int, allocators::extensible::Allocator<int, 8>> list_custom;

    auto factorial = [](size_t n){
        uint64_t res = 1;
        for(size_t i = 1; i <= n; i++) {
            res *= i;
        }
        return res;
    };

    for(size_t i = 0; i < 10; i++) {
        map_default[i] = factorial(i);
        map_custom[i] = factorial(i);
        list_default.append(i);
        list_custom.append(i);
    }

    std::cout << "Map with default allocator: ";
    for(size_t i = 0; i < 10; i++) {
        std::cout << map_default[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Map with custom allocator: ";
    for(size_t i = 0; i < 10; i++) {
        std::cout << map_custom[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Custom list with default allocator: ";
    for(auto value = list_default.getValue(); value.has_value(); value = list_default.getValue()) {
        std::cout << value.value() << " ";
    }
    std::cout << std::endl;

    std::cout << "Custom list with custom allocator: ";
    for(auto value = list_custom.getValue(); value.has_value(); value = list_custom.getValue()) {
        std::cout << value.value() << " ";
    }
    std::cout << std::endl;

    return 0;
}
