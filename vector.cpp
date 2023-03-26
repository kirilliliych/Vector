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
    //Vector<TestClass> test_vector()


    return 0;
}
