alter table objects drop column if exists coordinates_geom;
alter table objects add column coordinates_geom geometry;
update objects set coordinates_geom = ST_GeomFromText('POINT(' || DEGREES(ra) || ' ' || DEGREES(dec) || ')') where coordinates_geom is null;

select *, ST_AsText(ST_GeomFromText('POINT(' || DEGREES(ra) || ' ' || DEGREES(dec) || ')')) as geom2 from objects where object_id = 'NGC 224';

select objects.*, string_agg(denominations.name, ', ') as names , 
ST_Distance(coordinates_geom, (select coordinates_geom FROM objects where object_id = 'NGC 224') ) as dist
 from objects INNER JOIN denominations on objects.id = denominations.objects_id 
 group by objects.id
 order by dist asc 
 limit 20;
