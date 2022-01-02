#include "AprsSymbol.h"
#include "assert.h"

AprsSymbol::AprsSymbol() : _isPrimarySymbolTable(true), _code(INVALID_CODE) {}

AprsSymbol::AprsSymbol(char tableId, char symbol)
{
    setTableId(tableId);
    setSymbol(symbol);
}

void AprsSymbol::setTableId(char id)
{
    assert(id == PRIMARY_SYMBOL_TABLE || id == ALTERNATIVE_SYMBOL_TABLE);
    _isPrimarySymbolTable = id == PRIMARY_SYMBOL_TABLE ? true : false;
}
void AprsSymbol::setSymbol(char code)
{
    assert(code > 0 && code < 95);
    _code = code;
}

AprsSymbol &AprsSymbol::operator=(const AprsSymbol &loc)
{
    _isPrimarySymbolTable = loc._isPrimarySymbolTable;
    _code = loc._code;
    return (*this);
}

std::string AprsSymbol::encodeTableId() const
{
    return std::string(1, _isPrimarySymbolTable ? PRIMARY_SYMBOL_TABLE : ALTERNATIVE_SYMBOL_TABLE);
}

std::string AprsSymbol::encodeSymbol() const
{
    return std::string(1, _code);
}
