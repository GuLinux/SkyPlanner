#!/bin/bash
HOST="${1-postgres}"
PORT="${2-5432}"
DB_USER="${DB_USER-postgres}"
DB_NAME=${DB_NAME-skyplanner}"
pg_dump -h $HOST -p $PORT -U ${DB_USER} ${DB_NAME} -t objects -t denominations -t catalogues -a --column-inserts --inserts | xz -9e > objects_dump.sql.xz
