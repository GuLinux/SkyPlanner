select ST_GeomFromText('POINT(6.28144 -0.328995)')

select ST_GeomFromText('POINT(' || ra || ' ' || dec || ')') as geom from objects where id = 5059

select *, 

ST_Distance(( select ST_GeomFromText('POINT(' || ra || ' ' || dec || ')') as geom from objects where id = 5059
 ) , ST_GeomFromText('POINT(' || ra || ' ' || dec || ')') )as dist 

from objects INNER JOIN denominations on objects.id = denominations.objects_id order by dist asc LIMIT 20

alter table objects add column coordinates_geom geometry

select *, ST_AsText(ST_GeomFromText('POINT(' || DEGREES(ra) || ' ' || DEGREES(dec) || ')')) as geom2 from objects where object_id = 'NGC224';

update objects set coordinates_geom = ST_GeomFromText('POINT(' || DEGREES(ra) || ' ' || DEGREES(dec) || ')') where coordinates_geom is null;


select objects.*, string_agg(denominations.name, ', ') as names , 
ST_Distance(coordinates_geom, (select coordinates_geom FROM objects where id = 5059) ) as dist
 from objects INNER JOIN denominations on objects.id = denominations.objects_id 
 group by objects.id
 order by dist asc 
 limit 20;
