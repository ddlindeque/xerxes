#include "../xerxes_lib/machine.h"
#include "../xerxes_lib/cpu6502.h"
#include "../xerxes_lib/rom.h"
#include "../xerxes_lib/ram.h"
#include "monitor.h"
#include "emulator_debugger.h"
#include "console.h"

void show_root_commands(const std::string &msg = "")
{
    if (msg.empty()) {
        dave::console::show_operation("? (q)uit, (r)un, (s)tep, (l)ine, (w)atch");
    }
    else {
        dave::console::show_operation("? (q)uit, (r)un, (s)tep, (l)ine, (w)atch");
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
    auto rom1 = machine.install_device<dave::rom<0xA000, 0xBFFF>>();
    auto rom2 = machine.install_device<dave::rom<0xE000, 0xFFFF>>();
    machine.install_device<dave::ram<0x0100,0x01FF>>();
    auto monitor = machine.install_device<dave::monitor<0x0400, 0x07E7>>();

    // Set the boot address
    rom2->program(0xFFFE, 0);
    rom2->program(0xFFFF, 2);
    rom2->program(0xFFFC, 0);
    rom2->program(0xFFFD, 1);

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
                            auto addr = dave::console::get_addr_from_input("Address to watch: ");
                            debugger.add_watch(addr);
                        }
                        show_root_commands();
                        break;
                    case 'd':
                        if (true) {
                            auto addr = dave::console::get_addr_from_input("Address of watch to delete: ");
                            //debugger.delete_watch(addr);
                        }
                        show_root_commands();
                        break;
                    default:
                        show_root_commands("invalid watch command");
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

