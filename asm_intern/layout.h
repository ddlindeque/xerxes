#ifndef __LAYOUTH
#define __LAYOUTH

#include <vector>
#include <ostream>

#include "ast.h"

namespace dave
{
    bool tryLayout(std::vector<file> &files, REG16 &startAddress);
}

#endif