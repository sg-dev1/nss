/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "argparse.h"
#include <iostream>

ArgObject::ArgObject(std::string _argument, std::string _description)
    : argument(_argument), description(_description)
{
}

bool
ArgObject::isPresent()
{
    return this->_isPresent;
}

std::string
ArgObject::getArgument()
{
    return this->argument;
}

std::string
ArgObject::getValue()
{
    return this->value;
}

void
ArgObject::setValue(std::string newValue)
{
    this->_isPresent = true;
    this->value = newValue;
}

void
ArgObject::setPresent()
{
    this->_isPresent = true;
}

bool
ArgParser::parse(const std::vector<std::string> arguments)
{
    const std::string prefix("--");
    for (size_t i = 0; i < arguments.size(); i++) {
        std::string arg = arguments.at(i);
        if (arg.compare(0, prefix.size(), prefix) == 0) { // is option ?
            //std::cout << "Processing " << arg << "...\n";
            // look for option in programArgs
            bool found = false;
            int savedPos = -1;
            //std::cout << "programArgs size: " << this->programArgs.size() << "\n";
            for (size_t j = 0; j < this->programArgs.size(); j++) {
                //std::cout << "\tprogramArg: " << programArgs.at(j)->getArgument() << "\n";
                if (this->programArgs.at(j)->getArgument() == arg) {
                    found = true;
                    savedPos = j;
                }
            }
            if (!found) { // error
                std::cout << "Parsing Error: " << arg << " not found!\n";
                return false;
            }

            // look for an option argument
            if (i + 1 < arguments.size() &&
                arguments.at(i + 1).compare(0, prefix.size(),
                                            prefix)) {
                this->programArgs.at(savedPos)->setValue(std::string(arguments.at(i + 1)));
                i++;
            } else {
                this->programArgs.at(savedPos)->setPresent();
            }
        } else {
            // positional argument (e.g. required argument)
            this->positionalArgs.push_back(std::string(arg));
        }
    }

    return true;
}

void
ArgParser::add(std::shared_ptr<ArgObject> obj)
{
    //std::cout << "adding new ArgObject " << obj->getArgument() << "\n";
    this->programArgs.push_back(obj);
}

int
ArgParser::getPositionalArgumentCount()
{
    return this->positionalArgs.size();
}

std::string
ArgParser::getPositionalArgument(int pos)
{
    return this->positionalArgs.at(pos);
}
