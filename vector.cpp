#include <algorithm>
#include "bitvector.hpp"
#include "vector.hpp"


class TestClass
{
public:
    TestClass()
      : value_(0)
    {}

    TestClass(int value)
      : value_(value)
    {}

    ~TestClass()
    {
        value_ = -666;
        // value_ = false;
    }

    operator int() const
    {
        return value_;
    }

    int get_value() const
    {
        return value_;
    }

private:
    int value_ = 0;
};

void dump_elem(const TestClass &value)
{
    std::cout << value.get_value();
}


int main()
{
    // Vector<TestClass> v1{1212, 35, 231, 46, 88};

    // v1.dump(dump_elem);
    // v1.resize(8);
    // // v1.dump(dump_elem);
    // v1.resize(12);
    // v1.resize(15);
    // v1.dump(dump_elem);

    Vector<bool> v1{false, true, true, false};
    // v1.dump(dump_elem);

    std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    std::sort(v1.rbegin(), v1.rend());
    std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    v1.insert(v1.cbegin() + 3, 100);
    std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " ")); 
    std::cout << std::endl;
    v1.erase(v1.begin() + 2);
    // v1.resize(8);
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;
    // v1.shrink_to_fit();
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;
    // v1.reserve(12);
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;
    // v1.shrink_to_fit();
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;
    // v1.resize(12);
    // v1.dump(dump_elem);
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;
    // v1.shrink_to_fit();
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;
    // v1[1] = 11111;
    // std::copy(v1.begin(), v1.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;

    return 0;
}
