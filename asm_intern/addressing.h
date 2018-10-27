#ifndef __ADDRESSINGH
#define __ADDRESSINGH

#include "../xerxes_lib/common.h"
#include "ast.h"
#include <memory>
#include <unordered_map>

namespace dave
{
    bool tryDetermineAddressingMode(const std::unordered_map<std::string, value_range> &labels, const std::shared_ptr<expression> &parameter, addressing_mode &mode, std::shared_ptr<expression> &value);
}

#endif