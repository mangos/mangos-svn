RENAME TABLE `guild_charter` TO `petition`;
RENAME TABLE `guild_charter_sign` TO `petition_sign`;
ALTER TABLE `petition` ADD `type` int(10) unsigned NOT NULL default '0' AFTER `name`;
UPDATE `petition` SET `type`='9';
