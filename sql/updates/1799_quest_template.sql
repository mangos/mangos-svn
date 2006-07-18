ALTER TABLE `quest_template` 
    ADD COLUMN `RequiredRepFaction` int(11) unsigned NOT NULL default '0' AFTER `RequiredTradeskill`,
    ADD COLUMN `RequiredRepValue` int(11) unsigned NOT NULL default '0' AFTER `RequiredRepFaction1`,
    CHANGE `ReqKillMobId1` `ReqKillMobOrGOId1` int(11) NOT NULL default '0',
    CHANGE `ReqKillMobId2` `ReqKillMobOrGOId2` int(11) NOT NULL default '0',
    CHANGE `ReqKillMobId3` `ReqKillMobOrGOId3` int(11) NOT NULL default '0',
    CHANGE `ReqKillMobId4` `ReqKillMobOrGOId4` int(11) NOT NULL default '0';
