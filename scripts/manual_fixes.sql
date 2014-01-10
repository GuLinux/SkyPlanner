update objects set magnitude = 7.0 where id = (select objects_id from denominations where catalogue = 'NGC' AND "number" = '7293');
