//
// Created by Dieter Baron on 30.12.22.
//

#include <fstream>
#include <vector>

#include "Assembler.h"
#include "CPUParser.h"
#include "Command.h"
#include "Exception.h"

class xlr8_as: public Command {
public:
    xlr8_as(): Command(options, "file.s", "xlr8-as") {}

protected:
    void process() override;
    void create_output() override;
    std::string default_output_file() override;
    size_t minimum_arguments() override {return 1;}
    size_t maximum_arguments() override {return 1;}

private:
    ObjectFile source;

    static std::vector<Commandline::Option> options;
};


int main(int argc, char *argv[]) {
    auto command = xlr8_as();

    return command.run(argc, argv);
}


std::vector<Commandline::Option> xlr8_as::options = {
        Commandline::Option("cpu", "file", "read CPU definition from FILE")
};


std::string xlr8_as::default_output_file() {
    return std::filesystem::path(arguments.arguments[0]).stem().filename().string() + ".o";
}

void xlr8_as::process() {
    auto cpu_file = arguments.find_first("cpu");
    if (!cpu_file.has_value()) {
        throw Exception("missing --cpu option");
    }

    auto parser = CPUParser();
    auto cpu = parser.parse(cpu_file.value());

    auto assembler = Assembler(cpu);
    source = assembler.parse(arguments.arguments[0]);
}


void xlr8_as::create_output() {
    auto stream = std::ofstream(output_file);
    stream << source;
}
