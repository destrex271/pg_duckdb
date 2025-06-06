# MotherDuck Integration

## Connect with MotherDuck

`pg_duckdb` integrates with [MotherDuck][md] natively.
To enable this support you first need to [generate an access token][md-access-token].
Then you can enable it by simply using the `enable_motherduck` convenience method:

```sql
-- If not provided, the token will be read from the `motherduck_token` environment variable
-- If not provided, the default MD database name is `my_db`
CALL duckdb.enable_motherduck('<optional token>', '<optional MD database name>');
```

This convenience method creates a `motherduck` `SERVER` using the `pg_duckdb` Foreign Data Wrapper, which hosts the options for this integration. It also provides an `USER MAPPING` for the current user, which stores the provided MotherDuck token (if any).

You can refer to the [section](advanced-motherduck-configuration) below for more on the `SERVER` and `USER MAPPING` configuration.

### Non-supersuer configuration

If you want to use MotherDuck as a different user than a superuser you also have to configure:

```ini
duckdb.postgres_role = 'your_role_name'  # e.g. duckdb or duckdb_group
```

You also likely want to make sure that this role has `CREATE` permissions on the `public` schema in Postgres, because this is where the tables in the `main` schema are created in. You can grant these permissions as follows:

```sql
GRANT CREATE ON SCHEMA public TO {your_role_name};
-- So if you've configured the duckdb role above
GRANT CREATE ON SCHEMA public TO duckdb;
```

If you do this after starting postgres the initial sync of MotherDuck tables will probably have failed for the public schema. You can force a full resync of the tables by running:

```sql
select * from pg_terminate_backend((select pid from pg_stat_activity where backend_type = 'pg_duckdb sync worker'));
```

## Using `pg_duckdb` with MotherDuck

After doing the configuration (and possibly restarting Postgres). You can then you create tables in the MotherDuck database by using the `duckdb` [Table Access Method][tam] like this:

```sql
CREATE TABLE orders(id bigint, item text, price NUMERIC(10, 2)) USING duckdb;
CREATE TABLE users_md_copy USING duckdb AS SELECT * FROM users;
```

[tam]: https://www.postgresql.org/docs/current/tableam.html

Any tables that you already had in MotherDuck are automatically available in Postgres. Since DuckDB and MotherDuck allow accessing multiple databases from a single connection and Postgres does not, we map database+schema in DuckDB to a schema name in Postgres.

The default MotherDuck database will be easiest to use (see below for details), by default this is `my_db`.

## Advanced MotherDuck configuration

If you want to specify which MotherDuck database is your default database, then you need to configure MotherDuck using a `SERVER` and a `USER MAPPING` as such:

```sql
CREATE SERVER motherduck
TYPE 'motherduck'
FOREIGN DATA WRAPPER duckdb
OPTIONS (default_database '<your database>');

-- You may use `::FROM_ENV::` to have the token be read from the environment variable
CREATE USER MAPPING FOR CURRENT_USER SERVER motherduck OPTIONS (token '<your token>')
```

Note: with the `duckdb.enable_motherduck` convenience method above, you can simply do:
```sql
CALL duckdb.enable_motherduck('<token>', '<default database>');
```

## How DuckDB schemas are mapped to Postgres schemas

DuckDB and Postgres schema and database conventions are different. The mapping of database+schema to schema name is then done in the following way:

1. Each schema in your default MotherDuck database (see above on how to configure) is simply merged with the Postgres schema with the same name.
2. Except for the `main` DuckDB schema in your default database, which is merged with the Postgres `public` schema.
3. Tables in other databases are put into dedicated DuckDB-only schemas. These schemas are of the form `ddb$<duckdb_db_name>$<duckdb_schema_name>` (including the literal `$` characters).
4. Except for the `main` schema in those other databases. That schema should be accessed using the shorter name `ddb$<db_name>` instead.

An example of each of these cases is shown below:

```sql
INSERT INTO my_table VALUES (1, 'abc'); -- inserts into my_db.main.my_table
INSERT INTO your_schema.tab1 VALUES (1, 'abc'); -- inserts into my_db.your_schema.tab1
SELECT COUNT(*) FROM ddb$my_shared_db.aggregated_order_data; -- reads from my_shared_db.main.aggregated_order_data
SELECT COUNT(*) FROM ddb$sample_data$hn.hacker_news; -- reads from sample_data.hn.hacker_news
```

## Debugging issues

If some tables or schemas are not showing up as expected, it's best to check your Postgres log file. The background worker that automatically syncs tables might have run into an error when syncing some of the tables. It reports these failures in the log, often even including how the failure can be resolved.

[md]: https://motherduck.com/
[md-access-token]: https://motherduck.com/docs/key-tasks/authenticating-and-connecting-to-motherduck/authenticating-to-motherduck/#authentication-using-an-access-token
