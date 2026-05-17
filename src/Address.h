/*
Address.h --

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

#ifndef ADDRESS_H
#define ADDRESS_H

#include "Environment.h"
#include "Tokenizer.h"
#include <iostream>

/**
 * Represents an address, consisting of a bank and an address within that bank. The bank defaults to 0.
 */
class Address {
  public:
    /**
     * Constructs an address with the given bank and address.
     * 
     * @param bank The bank of the address.
     * @param address The address within the bank.
     */
    Address(uint64_t bank, uint64_t address): bank(bank), address(address) {}

    /**
     * Constructs an address with the given address and a default bank of 0.
     * 
     * @param address The address within the bank.
     */
    explicit Address(uint64_t address): address(address) {}

    /**
     * Constructs an address by parsing an expression from a tokenizer.
     * 
     * @param tokenizer The tokenizer to parse the expression from.
     * @param environment The environment to evaluate the expression in.
     */
    explicit Address(Tokenizer& tokenizer, std::shared_ptr<Environment> environment = {});

    /**
     * Serializes the address to a stream. The format is `address` if the bank is 0, and `bank:address` otherwise.
     */
    void serialize(std::ostream& stream) const;

    /**
     * Compares two addresses for equality.
     * 
     * @param other The address to compare with.
     * @return `true` if the addresses are equal, `false` otherwise.
     */
    bool operator==(const Address& other) const {return bank == other.bank && address == other.address;}

    /**
     * Compares two addresses for inequality.
     * 
     * @param other The address to compare with.
     * @return `true` if the addresses are not equal, `false` otherwise.
     */
    bool operator!=(const Address& other) const {return !(*this == other);}

    /**
     * Compares two addresses. Addresses are ordered by bank first, then by address within the bank.
     * 
     * @param other The address to compare with.
     * @return `true` if the address is less than the other, `false` otherwise.
     */
    bool operator<(const Address& other) const {return bank < other.bank || (bank == other.bank && address < other.address);}

    /// The bank of the address.
    uint64_t bank = 0;

    /// The address within the bank.
    uint64_t address = 0;
};

/**
 * Writes an address to a stream. The format is `address` if the bank is 0, and `bank:address` otherwise.
 */
std::ostream& operator<<(std::ostream& stream, Address address);

#endif // ADDRESS_H
