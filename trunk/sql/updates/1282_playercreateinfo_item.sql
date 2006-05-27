select * from item_template where inventorytype=15 or inventorytype=23 or inventorytype=24 or inventorytype=26  or name like "%shot%"

update item_template 
set rangedmodrange = 100
where inventorytype=15 or inventorytype=23 or inventorytype=24 or name like "%shot%"


