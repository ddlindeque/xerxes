#ifndef __PARSERH
#define __PARSERH

#include <string>
#include <istream>
#include <ostream>
#include "ast.h"

namespace dave
{
    bool tryParse(file &f);
}

#endif