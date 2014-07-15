BEGIN TRANSACTION;

update objects set magnitude = 7.0 where id = (select objects_id from denominations where catalogues_id = (select id from catalogues where code = 'NGC') AND "number" = '7293');
update catalogues set code = 'M' where name = 'Messier';
update catalogues set name = 'Common Name' where code = 'proper_name';
update catalogues set hidden = true, priority = -99, search_mode = 0 where code = 'proper_name';
update catalogues set hidden = true, priority = 0, search_mode = 2 where code = 'SAC_ASTERISMS';
update catalogues set hidden = true, priority = 0, search_mode = 2 where code = 'SAC_RED_STARS';
update catalogues set hidden = false, priority = -98, search_mode = 1 where code = 'M';
update catalogues set hidden = false, priority = -97, search_mode = 0 where code = 'NGC'; 
update catalogues set hidden = false, priority = -96, search_mode = 0 where code = 'IC';
update catalogues set hidden = false, priority = -95, search_mode = 0 where code = 'A';
update catalogues set hidden = false, priority = -94, search_mode = 0 where code = 'Arp';
update catalogues set hidden = false, priority = -93, search_mode = 0 where code = 'UGC';
insert into denominations (name, objects_id, catalogues_id) VALUES('UFO Galaxy', (select id from objects where object_id = 'NGC 2683'), 2);
insert into denominations (name, objects_id, catalogues_id) VALUES('Cat''s Eye Nebula', (select id from objects where object_id = 'NGC 6543'), 2)

END TRANSACTION;
