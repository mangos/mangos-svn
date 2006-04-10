-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 14, 2005 at 04:52 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- 
-- Dumping data for table `commands`
-- 

INSERT INTO `commands` VALUES ('help', 0, 'Syntax: .help <command name>\r\nDisplays help on a command.');
INSERT INTO `commands` VALUES ('acct', 0, 'Syntax: .acct\r\nDisplays the level of your account.');
INSERT INTO `commands` VALUES ('mount', 4, 'Syntax: .mount <mount number>\r\nMount from the mount number # \r\n(max=3)  lvl10=1 lvl15=2 lvl20=3');
INSERT INTO `commands` VALUES ('start', 4, 'Syntax: .start\r\nWarp to your start.');
INSERT INTO `commands` VALUES ('save', 0, 'Syntax: .save\r\nSave your character.');
INSERT INTO `commands` VALUES ('gps', 0, 'Syntax: .gps\r\nWill display the coordinates for your current position in the world.');
INSERT INTO `commands` VALUES ('modify', 4, 'Syntax: .modify # <new value>\r\n#  gold\r\n    mana\r\n    hp\r\n    level\r\n    speed\r\n    scale\r\n    mount');
INSERT INTO `commands` VALUES ('announce', 4, 'Syntax: .announce <Message to announce>\r\nSends a global message to all characters.');
INSERT INTO `commands` VALUES ('aura', 4, 'Syntax: .aura <aura number>\r\nTo test aura''s, can be unstable');
INSERT INTO `commands` VALUES ('learn', 4, 'Syntax: .learn <spell number>\r\nLearn a spell to your character');
INSERT INTO `commands` VALUES ('summon', 4, 'Syntax: .summon <character name>\r\nTeleport the user <character name> to you.');
INSERT INTO `commands` VALUES ('appear', 4, 'Syntax: .appear <character name>\r\nTeleport you to the user');
INSERT INTO `commands` VALUES ('kick', 4, 'Syntax: .kick <character name>\r\nForce to disconnect user (you can''t kick a >gm).');
INSERT INTO `commands` VALUES ('prog', 4, 'Syntax: .prog\r\nTeleports you to Programer''s Island.');
INSERT INTO `commands` VALUES ('guid', 4, 'Syntax: .guid\r\nWill display the GUID for the selected NPC.');
INSERT INTO `commands` VALUES ('AddSpawn', 4, 'Syntax: .AddSpawn <model number> <npc flag> <faction id> <level> <name>\r\nAllows you to spawn a NPC.\r\n<model number> = decimal model number');
INSERT INTO `commands` VALUES ('spawntaxi', 4, 'Syntax: .spawntaxi\r\nSpawns a taxi vendor at your current location.');
INSERT INTO `commands` VALUES ('delete', 4, 'Syntax: .delete\r\nDelete selected NPC.');
INSERT INTO `commands` VALUES ('name', 4, 'Syntax: .name <new name>\r\nChanges the name of the selected NPC. (Max 75 chars)');
INSERT INTO `commands` VALUES ('changelevel', 4, 'Syntax: .changelevel <new level>\r\nChange the level of selected NPC (max 99)');
INSERT INTO `commands` VALUES ('item', 4, 'Syntax: .item \r\nAllows you to assign an item to a vendor.');
INSERT INTO `commands` VALUES ('itemmove', 4, 'Syntax: .itemmove\r\nNOT WORKING');
INSERT INTO `commands` VALUES ('addmove', 4, 'Syntax: .move\r\nAdd your current location for move.');
INSERT INTO `commands` VALUES ('random', 4, 'Syntax: .random #\r\nSet random movement! 1=ranom(default), 0=path');
INSERT INTO `commands` VALUES ('run', 4, 'Syntax: .run #\r\nSet run or walk! 1=run, 0=walk(default)');
INSERT INTO `commands` VALUES ('anim', 4, '');
INSERT INTO `commands` VALUES ('animfreq', 4, '');
INSERT INTO `commands` VALUES ('commands', 4, 'Syntax: .commands\r\nWill display a list of available GM commands.');
INSERT INTO `commands` VALUES ('die', 4, 'Syntax: .die\r\nKills your character.');
INSERT INTO `commands` VALUES ('dismount', 0, 'Syntax: .dismount\r\nDismounts you, if you are mounted.');
INSERT INTO `commands` VALUES ('displayid', 4, 'Syntax: .displayid\r\nChanges the skin ID of a NPC.');
INSERT INTO `commands` VALUES ('factionid', 4, 'Syntax: .factionid\r\nChanges the faction ID of a NPC.');
INSERT INTO `commands` VALUES ('gmlist', 0, 'Syntax: .gmlist\r\nWill display a list of Game Masters online.');
INSERT INTO `commands` VALUES ('gmoff', 4, 'Syntax: .gmoff\r\nSwitches off <GM> prefix for your character.');
INSERT INTO `commands` VALUES ('gmon', 4, 'Syntax: .gmon\r\nSwitches on <GM> prefix for your character.');
INSERT INTO `commands` VALUES ('info', 0, 'Syntax: .info\r\nWill display the number of connected users.');
INSERT INTO `commands` VALUES ('morph', 4, 'Syntax: .morph\r\nChanges your skin.');
INSERT INTO `commands` VALUES ('go', 4, 'Syntax: .go X Y Z\r\nTeleports you to the coordinates given as X Y Z.');
INSERT INTO `commands` VALUES ('npcflag', 4, 'Syntax: .npcflag\r\nChanges the flag of the selected NPC.');
INSERT INTO `commands` VALUES ('security', 4, '');
INSERT INTO `commands` VALUES ('worldport', 4, 'Syntax: .worldport\r\nTeleports you around the world without the loading screen.');
INSERT INTO `commands` VALUES ('update', 4, '');
INSERT INTO `commands` VALUES ('addgrave', 4, 'Syntax: .addgrave\r\nWill add a graveyour location with the current position to the database.');
INSERT INTO `commands` VALUES ('addsh', 4, 'Syntax: .addsh\r\nSpawns a spirit healer on your current location. You wont see it, if you are not dead.');
INSERT INTO `commands` VALUES ('npcinfo', 4, '');
INSERT INTO `commands` VALUES ('demorph', 4, 'Syntax: .demorph\r\nWill change your skin back to the default skin.\r\n');
INSERT INTO `commands` VALUES ('revive', 4, 'Syntax: .revive\r\nWill revive the selected character.');
INSERT INTO `commands` VALUES ('addspw', 4, 'Syntax: .addspw <entry id>\r\nAllows to spawn a creature from a creature template using the given template id.\r\n<entry id> = decimal template id\r\n');
INSERT INTO `commands` VALUES ('loadscripts', 4, 'Syntax: .loadscripts <library_name>\r\nLoad script library with name <library_name> or default script library, and unload currently loaded script libarary.');
