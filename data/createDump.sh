#!/bin/bash
pg_dump -h localhost -p 5432 -U skyplanner skyplanner -t objects -t denominations -t catalogues -a --column-inserts --inserts | bzip2 > objects_dump.sql.bz2
