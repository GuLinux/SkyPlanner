#!/bin/bash
sqlite_file="$1"


if ! [ -r "$1" ]; then
  echo "SQLITE3 File $1 not existing; exiting"
  exit 1
fi

temp_file="$1"
sqlite3 "$temp_file" <<< "ALTER TABLE objects ADD COLUMN coordinates_geom TEXT;"

temp_sql="/tmp/$( basename "$0" ).$$.sql"
bzcat "$( dirname "$( dirname "$( readlink -f "$0")" )")/data/objects_dump.sql.bz2" | grep -v 'SET ' | grep -v 'pg_' > "$temp_sql"
pv -pet "$temp_sql" | sqlite3 "$temp_file"
rm "$temp_sql"

