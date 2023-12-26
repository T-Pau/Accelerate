#!/usr/bin/env python3

# convert-f256-microkernel.py.cc -- convert Microkernel include file
#
# Copyright (C) Dieter Baron
#
# The authors can be contacted at <accelerate@tpau.group>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. The names of the authors may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import sys


class LineIterator:
    def __init__(self):
        self.lines = []
        self.index = -1

    def add(self, line):
        self.lines.append(line)

    def reset(self):
        self.index = -1

    def next(self):
        self.index += 1
        if self.index < len(self.lines):
            return self.lines[self.index]
        else:
            return None

    def current(self):
        if self.index < 0 or self.index >= len(self.lines):
            return None
        else:
            return self.lines[self.index]

    def line_number(self):
        if self.index < 0:
            return None
        else:
            return self.index + 1  # line numbers start at 1


class Line:
    def __init__(self, line):
        line = line.rstrip(" \n")
        self.full_line = line
        comment_position = line.find(";")
        if comment_position == -1:
            self.comment = None
        else:
            self.comment = line[comment_position:]
            if comment_position == 0:
                line = ""
            else:
                line = line[:comment_position - 1]
        line = line.rstrip(" ")
        self.words = line.split()

    def __bool__(self):
        return True

    def __getitem__(self, item):
        return self.words[item]

    def __len__(self):
        return len(self.words)

    def empty(self):
        return len(self.words) == 0


def print_assignment(name, value, comment, file):
    if isinstance(value, int):
        value = "$" + hex(value)[2:]
    print(f"{namespace.prefix()}{name} = {value}", end="", file=file)
    if comment is not None:
        print(f"  {comment}", end="", file=file)
    print("", file=file)


class Text:
    def __init__(self, line):
        if line.comment is not None:
            self.text = line.comment
        else:
            self.text = ""

    def process(self, address, file):
        print(self.text, file=file)
        return address


class Address:
    def __init__(self, address):
        if address is None:
            self.address = None
        elif address.startswith("$"):
            self.address = int(address[1:], 16)
        else:
            self.address = int(address)

    def process(self, address, file):
        return self.address


class NamespaceChange:
    def __init__(self, component):
        self.component = component

    def process(self, address, file):
        if self.component is None:
            namespace.leave()
        else:
            namespace.enter(self.component)
        return address


class Assignment:
    def __init__(self, name, value, comment):
        self.name = name
        self.value = value
        self.comment = comment

    def process(self, address, file):
        print_assignment(self.name, self.value, self.comment, file)
        return address


class Skip:
    def __init__(self, amount):
        self.amount = int(amount)

    def process(self, address, file):
        if address is None:
            error(lines, "skip outside .virtual")
        else:
            return address + self.amount


class Member:
    def __init__(self, name, comment, size=None, structure=None):
        self.name = name
        self.comment = comment
        self.structure = structure
        self.size = size

    def process(self, address, file):
        if address is None:
            error(lines, "member outside of virtual")
            return address
        if self.structure is not None:
            structure = self.structure
            if not isinstance(structure, Structure):
                if structure in structures:
                    structure = structures[structure]
                else:
                    error(lines, f"unknown struct '{self.structure}'")
                    return address
            namespace.enter(self.name)
            address = structure.process(address, file)
            namespace.leave()
            return address
        else:
            print_assignment(self.name, address, self.comment, file)
            if self.size is None:
                error(lines, f"no size or struct given for {namespace.prefix()}{self.name}")
                return address
            return address + self.size


class Parsed:
    def __init__(self):
        self.members = []

    def add(self, member):
        self.members.append(member)


class Union:
    def __init__(self):
        self.members = []

    def add(self, member):
        self.members.append(member)

    def process(self, address, file):
        max_address = address
        for member in self.members:
            max_address = max(max_address, member.process(address, file))
        return max_address


class Structure:
    def __init__(self):
        self.members = []

    def add(self, member):
        self.members.append(member)

    def process(self, address, file):
        for member in self.members:
            address = member.process(address, file)
        return address


class Namespace:
    def __init__(self, namespace=None):
        self.components = []

    def empty(self):
        return len(self.components) == 0

    def enter(self, name):
        self.components.append(name)

    def leave(self):
        self.components = self.components[:-1]

    def clear(self):
        self.components = []

    def prefix(self):
        if self.empty():
            return ""
        else:
            return self.name() + "_"

    def name(self):
        return "_".join(self.components)


ok = True


def error(lines, message):
    if lines.line_number():
        print(f"{source}:{lines.line_number()}: {message}", file=sys.stderr)
    else:
        print(f"{source}: {message}", file=sys.stderr)
    ok = False


source = "f256-microkernel-api.asm"
destination = "f256-microkernel.s"
temp_name = destination + ".tmp"

lines = LineIterator()

with open(source, "r") as infile:
    while line := infile.readline():
        lines.add(Line(line))

structures = {}
parsed = Parsed()
parse_stack = [parsed]

namespace = Namespace()


def add_member(name, comment, size=None, structure=None):
    parse_stack[-1].add(Member(name, comment, size, structure))


while line := lines.next():
    if line.empty():
        parsed.add(Text(line))

    if len(line) == 1:
        if line[0] == ".endn":
            if namespace.empty():
                error(lines, "'.end' outside namespace")
            else:
                parsed.add(NamespaceChange(None))
                namespace.leave()
        elif line[0] == ".endv":
            parsed.add(Address(None))
        elif line[0] == ".ends":
            if len(parse_stack) == 1 or not isinstance(parse_stack[-1], Structure):
                error(lines, "'.ends' outside structure")
            else:
                parse_stack = parse_stack[:-1]
        elif line[0] == ".struct":
            structure = Structure()
            parse_stack[-1].add(structure)
            parse_stack.append(structure)
        elif line[0] == ".union":
            union = Union()
            parse_stack[-1].add(union)
            parse_stack.append(union)
        elif line[0] == ".endu":
            if len(parse_stack) == 1 or not isinstance(parse_stack[-1], Union):
                error(lines, "'.endu' outside union")
            else:
                parse_stack = parse_stack[:-1]

        else:
            error(lines, f"unrecognized line '{line.full_line}")
    elif len(line) == 2:
        if line[1] == ".namespace":
            namespace.enter(line[0])
            parsed.add(NamespaceChange(line[0]))
        elif line[0] == ".virtual":
            parsed.add(Address(line[1]))
        elif line[1] == ".struct":
            structure = Structure()
            structures[namespace.prefix() + line[0]] = structure
            parse_stack.append(structure)
        elif line[0] == ".word":
            parsed.add(Skip(2))
        elif line[0] == ".fill":
            parsed.add(Skip(line[1]))
        elif line[1] == ".ends":
            if len(parse_stack) == 1:
                error(lines, "'.ends' outside structure")
            else:
                parse_stack = parse_stack[:-1]
        else:
            error(lines, f"unrecognized line '{line.full_line}")
    elif len(line) == 3:
        if line[1] == "=":
            value = line[2]
            if not (value.isdigit() or value.startswith("$")):
                value = namespace.prefix() + value.replace(".", "_")
            parse_stack[-1].add(Assignment(line[0], value, line.comment))  # TODO: add structure name when in struct?
        elif line[1] == ".byte":
            add_member(line[0], line.comment, size=1)
        elif line[1] == ".word":
            add_member(line[0], line.comment, size=2)
        elif line[1] == ".dword":
            add_member(line[0], line.comment, size=4)
        elif line[1] == ".fill":
            add_member(line[0], line.comment, size=int(line[2]))
        elif line[1] == ".dstruct":
            name = line[2]
            if "." in name:
                name = name.replace(".", "_")
            else:
                name = namespace.prefix() + name
            add_member(line[0], line.comment, structure=name)
        else:
            error(lines, f"unrecognized line '{line.full_line}")

if len(parse_stack) != 1:
    error(lines, "unclosed struct/union")

if not namespace.empty():
    error(lines, "unclosed namespace")

if ok:
    with open(temp_name, "w") as file:
        print(f"; This file is automatically converted to Accelerate syntax by {sys.argv[0]}.", file=file)
        print(f"; Do not edit manually.", file=file)
        print(f"; Comments describing structures lost their spot in conversion.", file=file)
        print(f"", file=file)
        print(f".target \"f256\"", file=file)
        print(f".visibility public", file=file)
        print(f"", file=file)

        address = None
        namespace.clear()

        for member in parsed.members:
            address = member.process(address, file)

        namespace.clear()
        namespace.enter("kernel_event")
        structures["kernel_event_event_t"].process(0, file)

        namespace.clear()
        namespace.enter("kernel_time")
        structures["kernel_time_t"].process(0, file)

if ok:
    os.rename(temp_name, destination)
    sys.exit(0)
else:
    sys.exit(1)
