#include <cassert>
#include <iostream>


void print(const char *format)
{
    assert(format != nullptr);

    std::cout << format;
}

template <typename T, typename... ArgsT>
void print(const char *format, T arg, ArgsT... args)
{
    assert(format != nullptr);

    while (*format != '\0')
    {
        if (*format == '%')
        {
            if (*(format + 1) != '%')
            {
                std::cout << arg;

                print(format + 1, args...);

                return;
            }

            ++format;
        }

        std::putchar(*format);

        ++format;
    }
}

int main()
{
    int arg1 = 228;
    int arg2 = 1488;
    int arg3 = 1337;

    print( "here should be percent: %%, num1:% num2:% num3:%\n", arg1, arg2, arg3);

    return 0;    
}
