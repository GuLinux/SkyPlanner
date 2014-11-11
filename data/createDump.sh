#!/bin/bash
HOST="${1-localhost}"
PORT="${2-5432}"
pg_dump -h $HOST -p $PORT -U SkyPlanner SkyPlanner -t objects -t denominations -t catalogues -a --column-inserts --inserts | bzip2 > objects_dump.sql.bz2
