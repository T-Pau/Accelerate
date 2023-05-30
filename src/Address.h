//
// Created by Dieter Baron on 27.05.23.
//

#ifndef ADDRESS_H
#define ADDRESS_H

#include "Environment.h"
#include "Tokenizer.h"
#include <iostream>

class Address {
  public:
    Address(uint64_t bank, uint64_t address): bank(bank), address(address) {}
    explicit Address(uint64_t address): address(address) {}
    explicit Address(Tokenizer& tokenizer, const std::shared_ptr<Environment>& environment = {});

    void serialize(std::ostream& stream) const;

    uint64_t bank = 0;
    uint64_t address = 0;
};

std::ostream& operator<<(std::ostream& stream, Address address);

#endif // ADDRESS_H
