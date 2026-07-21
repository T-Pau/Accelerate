#ifdef IN_XLR8_BASE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_BASE_H
#ifndef HAD_XLR8_BASE_H
#define HAD_XLR8_BASE_H

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

#include <ostream>
#include <string>

#ifdef TRACE_TRANSLATION
#include <format>
#include <vector>

#define TRACE_BEGIN(what, format, ...) trace(true, what, false, format, ##__VA_ARGS__)
#define TRACE_END(what, format, ...) trace(false, what, false, format, ##__VA_ARGS__)
#define TRACE_BEGIN_PRINT(what, format, ...) trace(true, what, true, format, ##__VA_ARGS__)
#define TRACE_END_PRINT(what, format, ...) trace(false, what, true, format, ##__VA_ARGS__)
#define TRACE_BEGIN_INSTANCE(instance, what, format, ...) instance->trace(true, what, false, format, ##__VA_ARGS__)
#define TRACE_END_INSTANCE(instance, what, format, ...) instance->trace(false, what, false, format, ##__VA_ARGS__)
#define TRACE_BEGIN_INSTANCE_PRINT(instance, what, format, ...) instance->trace(true, what, true, format, ##__VA_ARGS__)
#define TRACE_END_INSTANCE_PRINT(instance, what, format, ...) instance->trace(false, what, true, format, ##__VA_ARGS__)
#else
#define TRACE_BEGIN(what, format, ...)
#define TRACE_END(what, format, ...)
#define TRACE_BEGIN_PRINT_THIS(what, format, ...)
#define TRACE_END_PRINT_THIS(what, format, ...)
#define TRACE_BEGIN_INSTANCE(instance, what, format, ...)
#define TRACE_END_INSTANCE(instance, what, format, ...)
#define TRACE_BEGIN_INSTANCE_PRINT(instance, what, format, ...)
#define TRACE_END_INSTANCE_PRINT(instance, what, format, ...)
#endif

/**
 * @brief Base class for class hierarchies.
 *
 * This class provides a base for class hierarchies, allowing for dynamic type identification and optional tracing of operations.
 *
 * It is used for Body, Entity and Expression hierarchies.
 */
class Base {
  public:
    virtual ~Base() = default;

    /**
     * @brief Get the name of the type of this object.
     *
     * @return The name of the type of this object.
     */
    std::string type_name() const;

    virtual void serialize(std::ostream& os) const {}

#ifdef TRACE_TRANSLATION
    template <typename... Args> void trace(bool begin, std::string_view what, bool print_this, std::format_string<Args...> format, Args&&... args) const { trace_implementation(begin, what, print_this, std::vformat(format.get(), std::make_format_args(args...))); }

#endif

  private:
#ifdef TRACE_TRANSLATION
    static std::vector<const Base*> trace_stack;

    void trace_implementation(bool begin, std::string_view what, bool print_this, std::string_view message) const;
#endif
};

std::ostream& operator<<(std::ostream& os, const Base& base);

#endif // HAD_XLR8_BASE_H
#undef IN_XLR8_BASE_H
