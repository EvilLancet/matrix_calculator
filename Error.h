#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <string>

class Error
{
public:
    enum Type
    {
        Syntax,
        Math
    } type;

    Error(std::string message, Type errorType)
    : type{errorType}
    {
        switch(errorType)
        {
        case Syntax:
            msg = "Syntax Error: " + message;
            break;
        case Math:
            msg = "Math Error: " + message;
            break;
        }
    }

    std::string what() { return msg; }

private:
    std::string msg;
};

#endif // ERROR_H_INCLUDED
