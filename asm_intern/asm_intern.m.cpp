#include <unordered_map>
#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <memory>
#include <vector>
#include <iomanip>

#include "code_generator.h"
#include "pc_code_generator.h"
#include "rom_code_generator.h"

#include "parser.h"
#include "logger.h"
#include "layout.h"

int main(int argc, char *argv[])
{
    dave::stm_logger slogger(std::cerr);
    dave::logger::def = &slogger;
    dave::logger::def->_filename.clear();
    dave::logger::def->_line_no = 0;

    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "intern [options]" << std::endl;
        std::cout << " -i  : specify input file (stdin if not specified)" << std::endl;
        std::cout << " -o  : specify output file (stdout if not specified)" << std::endl;
        std::cout << " -fmt: format, 'punchcard' or 'rom'" << std::endl;
        return 0;
    }

    std::unordered_map<std::string, std::vector<std::string> > args;
    {
        int i = 1;
        while(i < argc) {
            std::string a(argv[i]);
            if (!a.empty() && a[0] == '-') {
                // option
                i++;
                if (i < argc) {
                    auto f = args.emplace(a, std::vector<std::string>());
                    f.first->second.push_back(argv[i]);
                    i++;
                }
                else {
                    std::cerr << "Expected a value after '" << a << '\'' << std::endl;
                    return 1;
                }
            }
            else {
                // input
                auto f = args.emplace("-i", std::vector<std::string>());
                f.first->second.push_back(a);
                i++;
            }
        }
    }

    std::ostream *ostm;
    std::ofstream ofstm;
    auto f = args.find("-o");
    if (f != args.end()) {
        if (f->second.size() == 1) {
            ofstm.open(f->second[0]);
            if (!ofstm) {
                std::cerr << "Failure opening '" << f->second[0] << "' for output" << std::endl;
                return 1;
            }
            else {
                ostm = &ofstm;
            }
        }
        else {
            ostm = &std::cout;
        }
    }
    else {
        ostm = &std::cout;
    }

    std::unique_ptr<dave::code_generator> code_gen;
    f = args.find("-fmt");
    if (f != args.end()) {
        if (f->second.size() > 1) {
            std::cerr << "Cannot specify more than one output format" << std::endl;
            return 1;
        }
        else {
            if (f->second[0] == "punchcard") {
                code_gen.reset(new dave::pc_code_generator(*ostm));
            }
            else if (f->second[0] == "rom") {
                code_gen.reset(new dave::rom_code_generator(*ostm));
            }
            else {
                std::cerr << "Unsupported format '" << f->second[0] << "' encountered. Use 'punchcard' or 'rom'" << std::endl;
                return 1;
            }
        }
    }
    else {
        code_gen.reset(new dave::pc_code_generator(*ostm));
    }

    std::vector<dave::file> files;

    f = args.find("-i");
    if (f == args.end() || f->second.empty()) {
        std::cerr << "No input files" << std::endl;
        return 1;
    }
    else {
        for(auto &fn : f->second) {
            files.emplace_back();
            files.back().filename = fn;
            if (!dave::tryParse(files.back())) {
                return 1;
            }
        }
    }

    dave::REG16 startAddress;
    if (!dave::tryLayout(files, startAddress)) {
        return 1;
    }

    if (!code_gen->try_generate(files, startAddress)) {
        return 1;
    }

    return 0;
}