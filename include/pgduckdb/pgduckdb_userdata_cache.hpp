#pragma once

#include "pgduckdb/pg/declarations.hpp"

namespace pgduckdb {
void InitUserDataCache();
bool IsMotherDuckEnabled();
Oid MotherDuckPostgresUser();
void InvalidateUserDataCache();
Oid GetMotherduckForeignServerOid();
Oid GetMotherDuckUserMappingOid();
} // namespace pgduckdb
