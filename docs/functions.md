# pg_duckdb Functions

By default, functions without a schema listed below are installed into `public`. You can choose to install these functions to an alternate location by running `CREATE EXTENSION pg_duckdb WITH SCHEMA schema`.

Note: `ALTER EXTENSION pg_duckdb WITH SCHEMA schema` is not currently supported.

## Data Lake Functions

| Name | Description |
| :--- | :---------- |
| [`read_parquet`](#read_parquet) | Read a parquet file |
| [`read_csv`](#read_csv) | Read a CSV file |
| [`iceberg_scan`](#iceberg_scan) | Read an Iceberg dataset |
| [`iceberg_metadata`](#iceberg_metadata) | Read Iceberg metadata |
| [`iceberg_snapshots`](#iceberg_snapshots) | Read Iceberg snapshot information |

## DuckDB Administration Functions

| Name | Description |
| :--- | :---------- |
| [`duckdb.cache`](#cache) | Caches a Parquet or CSV file to disk |
| [`duckdb.install_extension`](#install_extension) | Installs a DuckDB extension |
| [`duckdb.raw_query`](#raw_query) | Runs a query directly against DuckDB (meant for debugging)|
| [`duckdb.recycle_ddb`](#recycle_ddb) | Force a reset the DuckDB instance in the current connection (meant for debugging) |

## Motherduck Functions

| Name | Description |
| :--- | :---------- |
| [`duckdb.force_motherduck_sync`](#force_motherduck_sync) | Forces a full resync of Motherduck databases and schemas to Postgres (meant for debugging) |

## Detailed Descriptions

#### <a name="read_parquet"></a>`read_parquet(path TEXT or TEXT[], /* optional parameters */) -> SETOF record`

Reads a parquet file, either from a remote location (via httpfs) or a local file.

Returns a record set (`SETOF record`). Functions that return record sets need to have their columns and types specified using `AS`. You must specify at least one column and any columns used in your query. For example:

```sql
SELECT COUNT(i) FROM read_parquet('file.parquet') AS (int i);
```

Further information:

* [DuckDB Parquet documentation](https://duckdb.org/docs/data/parquet/overview)
* [DuckDB httpfs documentation](https://duckdb.org/docs/extensions/httpfs/https.html)

##### Required Arguments

| Name | Type | Description |
| :--- | :--- | :---------- |
| path | text or text[] | The path, either to a remote httpfs file or a local file (if enabled), of the parquet file(s) to read. The path can be a glob or array of files to read. |

##### Optional Parameters

Optional parameters mirror [DuckDB's read_parquet function](https://duckdb.org/docs/data/parquet/overview.html#parameters). To specify optional parameters, use `parameter := 'value'`.

#### <a name="read_csv"></a>`read_csv(path TEXT or TEXT[], /* optional parameters */) -> SETOF record`

Reads a CSV file, either from a remote location (via httpfs) or a local file.

Returns a record set (`SETOF record`). Functions that return record sets need to have their columns and types specified using `AS`. You must specify at least one column and any columns used in your query. For example:

```sql
SELECT COUNT(i) FROM read_csv('file.csv') AS (int i);
```

Further information:

* [DuckDB CSV documentation](https://duckdb.org/docs/data/csv/overview)
* [DuckDB httpfs documentation](https://duckdb.org/docs/extensions/httpfs/https.html)

##### Required Arguments

| Name | Type | Description |
| :--- | :--- | :---------- |
| path | text or text[] | The path, either to a remote httpfs file or a local file (if enabled), of the CSV file(s) to read. The path can be a glob or array of files to read. |

##### Optional Parameters

Optional parameters mirror [DuckDB's read_csv function](https://duckdb.org/docs/data/csv/overview.html#parameters). To specify optional parameters, use `parameter := 'value'`.

Compatibility notes:

* `columns` is not currently supported.
* `nullstr` must be an array (`TEXT[]`).

#### <a name="iceberg_scan"></a>`iceberg_scan(path TEXT, /* optional parameters */) -> SETOF record`

Reads an Iceberg table, either from a remote location (via httpfs) or a local directory.

To use `iceberg_scan`, you must enable the `iceberg` extension:

```sql
SELECT duckdb.install_extension('iceberg');
```

Returns a record set (`SETOF record`). Functions that return record sets need to have their columns and types specified using `AS`. You must specify at least one column and any columns used in your query. For example:

```sql
SELECT COUNT(i) FROM iceberg_scan('data/iceberg/table') AS (int i);
```

Further information:

* [DuckDB Iceberg extension documentation](https://duckdb.org/docs/extensions/iceberg.html)

##### Required Arguments

| Name | Type | Description |
| :--- | :--- | :---------- |
| path | text | The path, either to a remote httpfs location or a local location (if enabled), of the Iceberg table to read. |

##### Optional Arguments

Optional parameters mirror DuckDB's `iceberg_scan` function based on the DuckDB source code. However, documentation on these parameters is limited. To specify optional parameters, use `parameter := 'value'`.

| Name | Type | Default | Description |
| :--- | :--- | :------ | :---------- |
| allowed_moved_paths | boolean | false | Ensures that some path resolution is performed, which allows scanning Iceberg tables that are moved. |
| mode | text | `''` | |
| metadata_compression_codec | text | `'none'` | |
| skip_schema_inference | boolean | false | |
| version | text | `'version-hint.text'` | |
| version_name_format | text | `'v%s%s.metadata.json,%s%s.metadata.json'` | |

#### <a name="iceberg_metadata"></a>`iceberg_metadata(path TEXT, /* optional parameters */) -> SETOF iceberg_metadata_record`

To use `iceberg_metadata`, you must enable the `iceberg` extension:

```sql
SELECT duckdb.install_extension('iceberg');
```

Return metadata about an iceberg table. Data is returned as a set of `icerberg_metadata_record`, which is defined as:

```sql
CREATE TYPE duckdb.iceberg_metadata_record AS (
  manifest_path TEXT,
  manifest_sequence_number NUMERIC,
  manifest_content TEXT,
  status TEXT,
  content TEXT,
  file_path TEXT
);
```

Further information:

* [DuckDB Iceberg extension documentation](https://duckdb.org/docs/extensions/iceberg.html)

##### Required Arguments

| Name | Type | Description |
| :--- | :--- | :---------- |
| path | text | The path, either to a remote httpfs location or a local location (if enabled), of the Iceberg table to read. |

##### Optional Arguments

Optional parameters mirror DuckDB's `iceberg_metadata` function based on the DuckDB source code. However, documentation on these parameters is limited. To specify optional parameters, use `parameter := 'value'`.

| Name | Type | Default | Description |
| :--- | :--- | :------ | :---------- |
| allowed_moved_paths | boolean | false | Ensures that some path resolution is performed, which allows scanning Iceberg tables that are moved. |
| metadata_compression_codec | text | `'none'` | |
| skip_schema_inference | boolean | false | |
| version | text | `'version-hint.text'` | |
| version_name_format | text | `'v%s%s.metadata.json,%s%s.metadata.json'` | |

#### <a name="iceberg_snapshots"></a>`iceberg_snapshots(path TEXT, /* optional parameters */) -> TODO`

TODO

#### <a name="cache"></a>`duckdb.cache(path TEXT, /* optional parameters */) -> bool`

TODO

#### <a name="install_extension"></a>`duckdb.install_extension(extension_name TEXT) -> bool`

TODO

#### <a name="raw_query"></a>`duckdb.raw_query(extension_name TEXT) -> void`

TODO

#### <a name="recycle_ddb"></a>`duckdb.recycle_ddb() -> void`

TODO

#### <a name="force_motherduck_sync"></a>`duckdb.force_motherduck_sync(drop_with_cascade BOOLEAN DEFAULT false)`

This is a procedure, so usage is `CALL`. This is mostly meant for debugging the automatic synchronization of MotherDuck tables, not for general use. If for some reason the synchronization is not working, you can use this to force a full resync of all MotherDuck databases and schemas to Postgres.