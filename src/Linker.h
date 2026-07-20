#ifdef IN_XLR8_LINKER_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_LINKER_H
#ifndef HAD_XLR8_LINKER_H
#define HAD_XLR8_LINKER_H

/*
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

#include "Target.h"
#include "UsedEntities.h"

/**
 * @brief The base class to translate a program or library.
 */
class Linker {
  public:
    Linker() = default;

    explicit Linker(Symbol name, const Target* target_) : module_(name) { set_target(target_); }

    virtual ~Linker() = default;

    /**
     * @brief Get the linker as a specific type.
     *
     * @tparam T The type to cast to.
     * @return The linker as the specified type, or nullptr if it is not of that type.
     */
    template <typename T> [[nodiscard]] T* as() { return dynamic_cast<T*>(this); }

    /**
     * @brief Get the linker as a specific type.
     *
     * @tparam T The type to cast to.
     * @return The linker as the specified type, or nullptr if it is not of that type.
     */
    template <typename T> [[nodiscard]] const T* as() const { return dynamic_cast<const T*>(this); }

    /**
     * @brief Check if the linker is of a specific type.
     *
     * @tparam T The type to check against.
     * @return true if the linker is of the specified type, false otherwise.
     */
    template <typename T> [[nodiscard]] bool is() const { return as<T>() != nullptr; }

    [[nodiscard]] const Module& module() const { return module_; }

    [[nodiscard]] Module& module() { return module_; }

    void set_target(const Target* new_target);

    /**
     * @brief If no target has been set yet, use the target from the module.
     *
     * @return `true` if a target is now set, `false` otherwise.
     */
    bool set_target_from_module();


    void link();
    void link_new();
    virtual void output(const std::filesystem::path& file_name) = 0;
    virtual const std::string& output_extension() const = 0;

    const Target* target = nullptr;

  protected:
    [[nodiscard]] virtual std::vector<Entity*> root_entities() = 0;

    virtual void link_sub() {}

    std::vector<Entity*> entities;

    Module module_;
};

#endif // HAD_XLR8_LINKER_H
#undef IN_XLR8_LINKER_H
