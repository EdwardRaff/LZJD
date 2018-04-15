// PostgreSQL includes
#include <postgres.h>
#include <utils/builtins.h> // in postgresql server includes, for text_to_cstring()

// Project includes
#include "pg_lzjd_helper.h"

PG_MODULE_MAGIC;

// 
// CREATE OR REPLACE FUNCTION lzjd_compare(TEXT, TEXT) RETURNS INTEGER AS 'lzjd_psql.so', 'pg_lzjd_compare' LANGUAGE 'C';
//

PG_FUNCTION_INFO_V1(pg_lzjd_compare);
Datum pg_lzjd_compare(PG_FUNCTION_ARGS);

Datum pg_lzjd_compare(PG_FUNCTION_ARGS) {
    text *arg1 = PG_GETARG_TEXT_P(0);
    text *arg2 = PG_GETARG_TEXT_P(1);
    char* hash1 = text_to_cstring(arg1);
    char* hash2 = text_to_cstring(arg2);
    
    int32 score = lzjd_similarity(hash1, hash2);

    pfree(hash1);
    pfree(hash2);

    PG_RETURN_INT32(score);
}
