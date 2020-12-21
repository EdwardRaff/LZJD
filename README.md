This is a fork of Edward Raff's [LZJD](https://github.com/EdwardRaff/LZJD). The added feature is a plugin for [Postgres](https://www.postgresql.org/) to allow similarity matching in SQL queries.

### Requirements:
* GCC
* CMake
* Boost libraries
* Postgres requirements:
  * Server development package, looks like `postgresql-server-dev-12`.

### Compile instructions:
* Check out the code
* `cd src`
* `mkdir build`
* `cmake ..`
* `make`

This creates:
* `liblzjd.shared.so`: Shared library for linking against.
* `liblzjd.static.a`: Static library for building against.
* `lzjd`: Command line application.
* If the Postgres server development package was installed, you should also have:
  * `lzjd_psql.so`: Postgresl plugin


### Installation instructions:
* Copy `lzjd` to `/usr/local/bin/` or similar, if desired.
* Copy `liblzjd.shared.so` to `/usr/local/lib/liblzjd.so`, or similar, if desired. Note the file name change.
* Copy `liblzjd.static.a` to `/usr/local/lib/liblzjd.a`, or similar, if desired. Note the file name change here too.
* For the Postgres plugin:
  * Run the command `pg_config --pkglibdir`, this is the installation directory.
  * Copy `lzjd_psql.so` to the directory shown in the above command. Should be `/usr/lib/postgresql/XX/lib/` where XX is the version number.
  * As the administrative user for your Postgres environment, run this SQL command to load the plugin: `CREATE OR REPLACE FUNCTION lzjd_compare(TEXT, TEXT) RETURNS INTEGER AS 'lzjd_psql.so', 'pg_lzjd_compare' LANGUAGE 'c';`. No restart required, the new function `lzjd_compare()` is available.