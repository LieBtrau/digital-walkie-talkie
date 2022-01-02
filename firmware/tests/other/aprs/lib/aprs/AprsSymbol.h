#pragma once

#include "Arduino.h"

class AprsSymbol
{
private:
    const char PRIMARY_SYMBOL_TABLE = '/';
    const char ALTERNATIVE_SYMBOL_TABLE = '\\';
    const char INVALID_CODE = 100;
    bool _isPrimarySymbolTable;
    char _code;

public:
    AprsSymbol();
    AprsSymbol(char tableId, char symbol);
    void setTableId(char id);
    void setSymbol(char code);
    AprsSymbol &operator=(const AprsSymbol &loc);
    std::string encodeTableId() const;
    std::string encodeSymbol() const;
};