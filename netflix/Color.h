#pragma once
#include <ostream>

namespace ansi {
    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& reset(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[0;37m";
    }

    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& foreground_green(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[32m";
    }

    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& foreground_red(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[31m";
    }

    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& foreground_yellow(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[93m";
    }
    
    //added a few more colors here for better looks if you're interested in colors you can find them here https://gist.github.com/vratiu/9780109
    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& foreground_purple(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[1;35m";
    }

    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& foreground_white(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[0;37m";
    }

    template < class CharT, class Traits >
    constexpr
        std::basic_ostream< CharT, Traits >& foreground_cyan(std::basic_ostream< CharT, Traits >& os)
    {
        return os << "\033[0;36m";
    }

}