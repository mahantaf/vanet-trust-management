#ifndef __CPP_STRING_H
#define __CPP_STRING_H

#include <string>

namespace omnetpp {
    class cpp_string: public std::string {
        //  cpp_string& operator=(const cpp_string& s)  {operator=(s.buf); return *this;}
        public: 
        cpp_string() {};

        cpp_string(std::string input):std::string(input) {
        }
    };
}
#endif