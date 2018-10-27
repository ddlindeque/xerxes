#include "logger.h"

namespace dave
{

logger *logger::def;

log_stream::log_stream(logger *logger)
: _logger(logger)
{
}

void log_stream::flush()
{
    _logger->log(_stm.str());
}

void operator<<(log_stream &ls, const end&)
{
    ls.flush();
}

}