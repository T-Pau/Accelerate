//
// Created by Dieter Baron on 27.05.23.
//

#include "Address.h"
#include "ExpressionParser.h"
#include "ParseException.h"

Address::Address(Tokenizer& tokenizer, const std::shared_ptr<Environment>& environment) {
    auto expression = ExpressionParser(tokenizer).parse();
    if (environment) {
        expression.evaluate(environment);
    }
    auto value = expression.value();
    if (!value.has_value() || !value->is_unsigned()) {
        throw ParseException(expression.location(), "constant unsigned integer expression expected");
    }

    auto token = tokenizer.next();

    if (token == Token::colon) {
        bank = value->unsigned_value();
        expression = ExpressionParser(tokenizer).parse();
        if (environment) {
            expression.evaluate(environment);
        }
        value = expression.value();
        if (!value.has_value() || !value->is_unsigned()) {
            throw ParseException(expression.location(), "constant unsigned integer expression expected");
        }
    }
    else {
        tokenizer.unget(token);
    }
    address = value->unsigned_value();
}


std::ostream& operator<<(std::ostream& stream, Address address) {
    address.serialize(stream);
    return stream;
}

void Address::serialize(std::ostream &stream) const {
    if (bank > 0) {
        stream << "$" << std::hex << bank << ":";
    }
    stream << "$" << std::hex << address;
}
