//
// Created by Dieter Baron on 30.12.22.
//

#include <fstream>
#include <vector>

#include "Assembler.h"
#include "Command.h"
#include "Exception.h"
#include "TargetParser.h"
#include "Util.h"

class xlr8_as: public Command {
public:
    xlr8_as(): Command(options, "file.s", "xlr8-as") {}

protected:
    void process() override;
    void create_output() override;
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
        Commandline::Option("target", "file", "read target definition from FILE")
};


void xlr8_as::process() {
    auto target_file = arguments.find_first("target");
    if (!target_file.has_value()) {
        throw Exception("missing --target option");
    }

    output_file = default_output_filename(arguments.arguments[0], "o");

    auto target = TargetParser().parse(target_file.value());

    auto assembler = Assembler(target.cpu);
    source = assembler.parse(arguments.arguments[0]);
}


void xlr8_as::create_output() {
    auto stream = std::ofstream(output_file.value());
    stream << source;
}
