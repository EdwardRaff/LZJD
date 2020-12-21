CREATE OR REPLACE FUNCTION lzjd_compare(TEXT, TEXT) RETURNS INTEGER AS 'lzjd_psql.so', 'pg_lzjd_compare' LANGUAGE 'c';
