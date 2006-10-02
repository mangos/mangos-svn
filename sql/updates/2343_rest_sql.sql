ALTER TABLE `character` 
    ADD `logout_time` int(11) NOT NULL DEFAULT '0',
    ADD `is_logout_resting` int(11) NOT NULL DEFAULT '0',
    ADD `rest_bonus` FLOAT NOT NULL DEFAULT '0';
DROP TABLE IF EXISTS `areatrigger_city`;
ALTER TABLE `areatrigger_template` 
    DROP `name`,
    DROP `target_map`,
    ADD `name` text AFTER `id`,
    ADD `trigger_map` int(11) unsigned NOT NULL DEFAULT '0' AFTER `name`,
    ADD `trigger_postion_x` FLOAT NOT NULL DEFAULT '0' AFTER `trigger_map`,
    ADD `trigger_position_y` FLOAT NOT NULL DEFAULT '0' AFTER `trigger_postion_x`,
    ADD `trigger_position_z` FLOAT NOT NULL DEFAULT '0' AFTER `trigger_position_y`,
    ADD `target_map` int(11) unsigned NOT NULL DEFAULT '0' AFTER `trigger_position_z`;