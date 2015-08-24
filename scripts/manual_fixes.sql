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

update denominations set objects_id = (select id from objects where object_id = 'NGC 770'), comment = comment || ', Arp 78B' WHERE other_catalogues = 'NGC 0770'
update denominations set objects_id = (select id from objects where object_id = 'NGC 772'), comment = comment || ', Arp 78A' WHERE other_catalogues = 'NGC 0772'

update denominations set objects_id = (select id from objects where object_id = replace(denominations.other_catalogues, ' 0', ' ') ) where denominations.id in (
  select denominations.id from denominations inner join objects on objects.object_id = replace(denominations.other_catalogues, ' 0', ' ') where other_catalogues like '%C 0%'
)


update denominations set objects_id = ( select objects_id from denominations where name = 'NGC 3184' ) WHERE name = 'MCG+07-21-037';
insert into denominations 
       (number, name      , objects_id                                                    , other_catalogues, comment                 , catalogues_id) 
VALUES (3180  , 'NGC 3180', (select objects_id from denominations where name = 'NGC 3184'), 'NGC 3184'      , 'HII region of NGC 3184', (select id from catalogues WHERE name = 'NGC') )

insert into denominations 
       (number, name      , objects_id                                                    , other_catalogues, comment                 , catalogues_id) 
VALUES (3181  , 'NGC 3181', (select objects_id from denominations where name = 'NGC 3184'), 'NGC 3184'      , 'HII region of NGC 3184', (select id from catalogues WHERE name = 'NGC') )



insert into denominations
       (name      , objects_id                                                    , other_catalogues,  catalogues_id)
VALUES ('The Grasshopper', (select objects_id from denominations where name = 'Arp 55' group by objects_id), 'UGC 04881'      , (select id from catalogues WHERE code = 'proper_name') )



-- remove multiple spaces
update denominations set name = regexp_replace(name, '\s{2,}', ' ') where name like '%  %'


-- duplicates UGC-ARP
update denominations den set objects_id = (select objects_id from denominations where name = replace(den.other_catalogues, ' 0', ' '))
  where den.id in (
select d.id
  from denominations d left join objects on objects.id = (select objects_id from denominations where name = replace(d.other_catalogues, ' 0', ' '))
  where d.other_catalogues like 'UGC 0%' AND NOT d.other_catalogues like '%;%' AND objects.id <> d.objects_id
  )
  

-- Cleanup orphan objects
delete from astro_session_object where objects_id in (select objects.id from objects left join denominations ON objects.id = denominations.objects_id where denominations.id is null);
delete from objects where id in (select objects.id from objects left join denominations ON objects.id = denominations.objects_id where denominations.id is null)





END TRANSACTION;
