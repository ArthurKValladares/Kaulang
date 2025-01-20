#pragma once

#include "parser.h"

struct Resolver {
    void resolve(Stmt* stmts, u64 stmts_len);
};