#include "../xerxes_lib/machine.h"
#include "../xerxes_lib/cpu6502.h"
#include "../xerxes_lib/rom.h"
#include "../xerxes_lib/ram.h"
#include "../xerxes_lib/punchcardreader.h"
#include "monitor.h"
#include "emulator_debugger.h"
#include "console.h"

void show_root_commands(const std::string &msg = "")
{
    dave::console::show_operation("? (q)uit, (r)un, (s)tep, (l)ine, (w)atch, (b)reak, (p)ause");
    if (!msg.empty()) {
        dave::console::alert(msg);
    }
}

template<typename TRom> void initialize_kernel_rom(TRom *rom)
{
    rom->program(0xE000, 0xA2);
    rom->program(0xE001, 0xFF);
    rom->program(0xE002, 0x9A);
    rom->program(0xE003, 0x20);
    rom->program(0xE004, 0x00);
    rom->program(0xE005, 0xE1);
    rom->program(0xE006, 0x58);
    rom->program(0xE007, 0xEA);
    rom->program(0xE008, 0x4C);
    rom->program(0xE009, 0x07);
    rom->program(0xE00A, 0xE0);

    rom->program(0xE100, 0xA9);
    rom->program(0xE101, 0x20);
    rom->program(0xE102, 0x85);
    rom->program(0xE103, 0x04);
    rom->program(0xE104, 0xA9);
    rom->program(0xE105, 0xE8);
    rom->program(0xE106, 0x85);
    rom->program(0xE107, 0x05);
    rom->program(0xE108, 0xA9);
    rom->program(0xE109, 0x21);
    rom->program(0xE10A, 0x85);
    rom->program(0xE10B, 0x06);
    rom->program(0xE10C, 0xA9);
    rom->program(0xE10D, 0xE8);
    rom->program(0xE10E, 0x85);
    rom->program(0xE10F, 0x07);
    rom->program(0xE110, 0xA9);
    rom->program(0xE111, 0x30);
    rom->program(0xE112, 0x85);
    rom->program(0xE113, 0x08);
    rom->program(0xE114, 0xA9);
    rom->program(0xE115, 0xE8);
    rom->program(0xE116, 0x85);
    rom->program(0xE117, 0x09);
    rom->program(0xE118, 0xA9);
    rom->program(0xE119, 0x40);
    rom->program(0xE11A, 0x85);
    rom->program(0xE11B, 0x0A);
    rom->program(0xE11C, 0xA9);
    rom->program(0xE11D, 0xE8);
    rom->program(0xE11E, 0x85);
    rom->program(0xE11F, 0x0B);
    rom->program(0xE120, 0xA9);
    rom->program(0xE121, 0x50);
    rom->program(0xE122, 0x85);
    rom->program(0xE123, 0x0C);
    rom->program(0xE124, 0xA9);
    rom->program(0xE125, 0xE8);
    rom->program(0xE126, 0x85);
    rom->program(0xE127, 0x0D);
    rom->program(0xE128, 0xA9);
    rom->program(0xE129, 0x01);
    rom->program(0xE12A, 0x8D);
    rom->program(0xE12B, 0x2F);
    rom->program(0xE12C, 0xD0);
    rom->program(0xE12D, 0x20);
    rom->program(0xE12E, 0x40);
    rom->program(0xE12F, 0xE1);
    rom->program(0xE130, 0x60);

    rom->program(0xE140, 0xA9);
    rom->program(0xE141, 0x02);
    rom->program(0xE142, 0x8D);
    rom->program(0xE143, 0x2F);
    rom->program(0xE144, 0xD0);
    rom->program(0xE145, 0x60);

    rom->program(0xE800, 0x20);
    rom->program(0xE801, 0x10);
    rom->program(0xE802, 0xE8);
    rom->program(0xE803, 0x58);
    rom->program(0xE804, 0x40);

    rom->program(0xE810, 0xAD);
    rom->program(0xE811, 0x30);
    rom->program(0xE812, 0xD0);
    rom->program(0xE813, 0x0A);
    rom->program(0xE814, 0xAA);
    rom->program(0xE815, 0x7C);
    rom->program(0xE816, 0x04);

    rom->program(0xE820, 0x60);

    rom->program(0xE821, 0xAD);
    rom->program(0xE822, 0x31);
    rom->program(0xE823, 0xD0);
    rom->program(0xE824, 0xA4);
    rom->program(0xE825, 0x03);
    rom->program(0xE826, 0x91);
    rom->program(0xE827, 0x0E);
    rom->program(0xE828, 0xE6);
    rom->program(0xE829, 0x03);
    rom->program(0xE82A, 0x20);
    rom->program(0xE82B, 0x40);
    rom->program(0xE82C, 0xE1);
    rom->program(0xE82D, 0x60);

    rom->program(0xE830, 0xAD);
    rom->program(0xE831, 0x31);
    rom->program(0xE832, 0xD0);
    rom->program(0xE833, 0x85);
    rom->program(0xE834, 0x0E);
    rom->program(0xE835, 0x20);
    rom->program(0xE836, 0x40);
    rom->program(0xE837, 0xE1);
    rom->program(0xE838, 0x60);

    rom->program(0xE840, 0xAD);
    rom->program(0xE841, 0x31);
    rom->program(0xE842, 0xD0);
    rom->program(0xE843, 0x85);
    rom->program(0xE844, 0x0F);
    rom->program(0xE845, 0xA9);
    rom->program(0xE846, 0x00);
    rom->program(0xE847, 0x85);
    rom->program(0xE848, 0x03);
    rom->program(0xE849, 0x20);
    rom->program(0xE84A, 0x40);
    rom->program(0xE84B, 0xE1);
    rom->program(0xE84C, 0x60);

    rom->program(0xE850, 0x58);
    rom->program(0xE851, 0x6C);
    rom->program(0xE852, 0x0E);

    rom->program(0xFFFA, 0x00);
    rom->program(0xFFFB, 0x00);
    rom->program(0xFFFC, 0x00);
    rom->program(0xFFFD, 0xE0);
    rom->program(0xFFFE, 0x00);
    rom->program(0xFFFF, 0xE8);
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
    auto kernel_rom = machine.install_device<dave::rom<0xE000, 0xFFFF>>();
    auto monitor = machine.install_device<dave::monitor<0x0400, 0x07E7>>();
    auto punchcardreader = machine.install_device<dave::punchcardreader<0xD02F, 0xD030, 0xD031>>("/Users/dlindeque/personal/xerxes/data/editor.pc");

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

