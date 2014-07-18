select 'angular_size' as desc, objects.id, string_agg(name, ';') from objects 
  INNER JOIN denominations on objects.id = denominations.objects_id
  WHERE angular_size = 0 OR angular_size IS NULL
  group by objects.id
UNION
select 'magnitude'as desc, objects.id, string_agg(name, ';') from objects
  INNER JOIN denominations on objects.id = denominations.objects_id
  WHERE magnitude > 90 OR magnitude IS NULL
  group by objects.id

