#ifdef IN_XLR8_LIBRARY_LINKER_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_LIBRARY_LINKER_H
#ifndef HAD_XLR8_LIBRARY_LINKER_H
#define HAD_XLR8_LIBRARY_LINKER_H

/*
Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

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

#include "Linker.h"

class LibraryLinker : public Linker {
  public:
    LibraryLinker(Symbol name, const Target* target = nullptr) : Linker(name, target) {}

    void output(const std::filesystem::path& file_name) override;

    const std::string& output_extension() const override { return library_extension; }

  protected:
    void link_sub() override;

  private:
    template <typename T> void output_entities(std::ostream& stream) {
        auto entities = sorted(module().get_entities<T>(), [](const T* a, const T* b) { return a->name < b->name; });
        for (const auto& entity : entities) {
            entity->serialize(stream);
        }
    }

    static const unsigned int format_version_major;
    static const unsigned int format_version_minor;
    static const std::string& library_extension;
};

#endif // HAD_XLR8_LIBRARY_LINKER_H
#undef IN_XLR8_LIBRARY_LINKER_H
