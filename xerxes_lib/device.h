#ifndef __DEVICEH
#define __DEVICEH

#include "common.h"

namespace dave
{
    class system_bus;

    class device {
    protected:
        system_bus *_bus;
    public:
        device() = delete;
        device(const device&) = delete;
        device(device &&) = delete;
        virtual ~device();

        auto operator =(const device&)->device& = delete;
        auto operator =(device &&)->device& = delete;

        device(system_bus *bus);

        virtual void powerup() {}
        virtual void nop() = 0;
        virtual void write(const REG16 &address, const REG8 *data) = 0;
        virtual void read(const REG16 &address, REG8 *dest) = 0;
    };
}

#endif