/*
Command.h -- 

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMAND_H
#define COMMAND_H

#include <limits>
#include <string>

#include "Commandline.h"

class Command {
public:
    Command(const std::vector<Commandline::Option>& options, std::string arguments, const std::string& name);
    virtual ~Command() = default;

    int run(int argc, char * const argv[]);

    std::string program_name;

protected:
    virtual void process() = 0;
    virtual void create_output() = 0;
    virtual size_t minimum_arguments() {return 0;}
    virtual size_t maximum_arguments() {return std::numeric_limits<size_t>::max();}

    void set_output_file(const std::string& file_name, const std::string& extension);

    Commandline commandline;
    ParsedCommandline arguments;
    std::optional<std::string> output_file;

private:
    static std::vector<Commandline::Option> standard_options;
    static std::string header;
    static std::string footer;
    static std::string version;
};


#endif // COMMAND_H
