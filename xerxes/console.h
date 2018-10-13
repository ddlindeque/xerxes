#ifndef __CONSOLEH
#define __CONSOLEH

#include <string>
#include "../xerxes_lib/common.h"
#include "../xerxes_lib/system_bus.h"

namespace dave
{
    class console {
    public:
        static void initialize();
        static void teardown();

        static void reset_cursor();
        static void draw_screen();

        static void update_cpu_register(const std::string &name, const bool &value);
        static void update_cpu_register(const std::string &name, const uint8_t &value);
        static void update_cpu_register(const std::string &name, const uint16_t &value);

        static int getkey();
        static bool try_getkey(int &key);

        static void update_char_on_virtual_monitor(const REG16 &addr, const REG8 &data);
        static void show_operation(const std::string &op);
        static void update_ticks(const size_t ticks);
        static void alert(const std::string &msg);
        static void clear_alert();

        static void clear_bus();
        static void add_bus(const REG16 &addr, const REG8 *data);

        static void report_pc(system_bus *bus, const REG16 &addr);
        static void report_s(system_bus *bus, const REG8 &value);

        static void update_nmi_line(bool value);
        static void update_irq_line(bool value);
        static void update_reset_line(bool value);

        static REG16 get_addr_from_input(const std::string &label);

        static void clear_watches();
        static void add_watch(const REG16 &addr, const REG8 &cur_value, bool value_changed);

        static void clear_breakpoints();
        static void add_bus_breakpoint(const REG16 &addr);
        static void add_pc_breakpoint(const REG16 &addr);

        static void report_punchcardreader_status(bool irqHigh, bool nextByteRequested, REG8 status, REG8 byteInBuffer);
        static void set_break_config(bool break_on_nmi, bool break_on_irq, bool break_on_reset);
    };
}

#endif