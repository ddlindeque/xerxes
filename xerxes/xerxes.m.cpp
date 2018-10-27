#include "../xerxes_lib/machine.h"
#include "../xerxes_lib/cpu6502.h"
#include "../xerxes_lib/rom.h"
#include "../xerxes_lib/ram.h"
#include "../xerxes_lib/punchcardreader.h"
#include "monitor.h"
#include "emulator_debugger.h"
#include "console.h"
#include "../software/romv2.h"

void show_root_commands(const std::string &msg = "")
{
    dave::console::show_operation("? (q)uit, (r)un, (s)tep, (l)ine, (w)atch, (b)reak, (p)ause");
    if (!msg.empty()) {
        dave::console::alert(msg);
    }
}

int main()
{
    dave::console::initialize();

    dave::console::draw_screen();

    dave::emulator_debugger debugger;

    dave::machine machine(&debugger);

    machine.install_cpu<dave::cpu6502>();
    
    machine.install_device<dave::ram<0x0000,0x00FF>>(); // Page Zero
    machine.install_device<dave::ram<0x0100,0x01FF>>(); // Stack
    machine.install_device<dave::ram<0x0200, 0x9FFF>>(); // General RAM
    machine.install_device<dave::ram<0xC000, 0xCFFF>>(); // General RAM
    auto kernel_rom = machine.install_device<dave::rom<0xE000, 0xFFFF>>();
    auto monitor = machine.install_device<dave::monitor<0x0400, 0x07E7>>();
    auto punchcardreader = machine.install_device<dave::punchcardreader<0xD02F, 0xD030, 0xD031>>("/Users/dlindeque/personal/xerxes/software/software.pc");

    initialize_kernel_rom(kernel_rom);

    machine.powerup();
    machine.report_cpu_status();
    while(true) {
        auto key = dave::console::getkey();
        dave::console::clear_bus();
        switch(key) {
            case 'q':
                dave::console::teardown();
                return 0;
            case 'r':
                dave::console::show_operation("run");
                debugger._break_after_instruction = false;
                machine.run();
                debugger.refresh_watches();
                show_root_commands();
                break;
            case 's':
                dave::console::show_operation("step");
                debugger._break_after_instruction = true;
                machine.run();
                debugger.refresh_watches();
                break;
            case 'l':
                dave::console::show_operation("? toggle line (i)rq, (n)mi, (r)eset");
                key = dave::console::getkey();
                switch(key) {
                    case 'i':
                        machine.toggle_irq();
                        break;
                    case 'n':
                        machine.toggle_nmi();
                        break;
                    case 'r':
                        machine.toggle_reset();
                        break;
                    default:
                        show_root_commands("invalid line");
                        break;
                }
                break;
            case 'w':
                dave::console::show_operation("? (a)dd, (d)elete");
                key = dave::console::getkey();
                switch(key) {
                    case 'a':
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("Address to watch");
                            debugger.add_watch(addr);
                        }
                        show_root_commands();
                        break;
                    case 'd':
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("Address of watch to delete");
                            debugger.delete_watch(addr);
                        }
                        show_root_commands();
                        break;
                    default:
                        show_root_commands("invalid watch command");
                        break;
                }
                break;
            case 'p':
                dave::console::show_operation("? (p)c value, (b)us, delete p(c), delete b(u)s, on (n)mi, on (i)rq, on (r)eset");
                key = dave::console::getkey();
                switch(key) {
                    case 'p': // Add a PC address breakpoint
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("When PC becomes (address)");
                            debugger.add_pc_breakpoint(addr);
                        }
                        show_root_commands();
                        break;
                    case 'b': // Add a bus address breakpoint
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("Address change to break on (address)");
                            debugger.add_bus_breakpoint(addr);
                        }
                        show_root_commands();
                        break;
                    case 'c': // Delete a PC breakpoint
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("PC Address to delete");
                            debugger.delete_pc_breakpoint(addr);
                        }
                        show_root_commands();
                        break;
                    case 'u': // Delete a bus breakpoint
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("Buss Address to delete");
                            debugger.delete_bus_breakpoint(addr);
                        }
                        show_root_commands();
                        break;
                    case 'n':
                        debugger.toggle_break_on_nmi();
                        show_root_commands();
                        break;
                    case 'i':
                        debugger.toggle_break_on_irq();
                        show_root_commands();
                        break;
                    case 'r':
                        debugger.toggle_break_on_reset();
                        show_root_commands();
                        break;
                    default:
                        show_root_commands("invalid pause command");
                        break;
                }
                break;
            default:
                show_root_commands("Invalid command");
                break;
        }
        machine.report_cpu_status();
    }
    
    dave::console::teardown();

    return 0;
}

