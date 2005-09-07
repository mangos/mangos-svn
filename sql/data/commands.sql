-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 07, 2005 at 07:43 AM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- 
-- Dumping data for table `commands`
-- 

INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('help', 0, 'Syntax: .help <command name>\r\nDisplays help on a command.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('acct', 0, 'Syntax: .acct\r\nDisplays the level of your account');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('mount', 4, 'Syntax: .mount <mount number>\r\nMount from the mount number # \r\n(max=3)  lvl10=1 lvl15=2 lvl20=3');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('start', 4, 'Syntax: .start\r\nWarp to your start.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('save', 0, 'Syntax: .save\r\nSave your character.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('gps', 0, 'Syntax: .gps\r\nGives your coordinates.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('modify', 4, 'Syntax: .modify # <new value>\r\n#  gold\r\n    mana\r\n    hp\r\n    level\r\n    speed\r\n    scale\r\n    mount');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('announce', 4, 'Syntax: .announce <Message to announce>\r\nSends a Global message.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('aura', 4, 'Syntax: .aura <aura number>\r\nTo test aura''s, can be unstable');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('learn', 4, 'Syntax: .learn <spell number>\r\nLearn a spell to your character');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('summon', 4, 'Syntax: .summon <character name>\r\nTeleport the user to you');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('appear', 4, 'Syntax: .appear <character name>\r\nTeleport you to the user');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('kick', 4, 'Syntax: .kick <character name>\r\nForce to disconnect user (you can''t kick a >gm).');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('prog', 4, 'Syntax: .prog\r\nTeleports you to Programer''s Island.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('guid', 4, 'Syntax: .guid\r\nGet GUID from selected NPC');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('AddSpawn', 4, 'Syntax: .AddSpawn <model number> <npc flag> <faction id> <level> <name>\r\nAllows you to spawn a NPC.\r\n<model number> = decimal model number\r\n');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('spawntaxi', 4, 'Syntax: .spawntaxi\r\nSpawns a taxi vendor at your current location.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('delete', 4, 'Syntax: .delete\r\nDelete selected NPC.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('name', 4, 'Syntax: .name <new name>\r\nChanges the name of the selected NPC. (Max 75 chars)');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('changelevel', 4, 'Syntax: .changelevel <new level>\r\nChange the level of selected NPC (max 99)');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('item', 4, 'Syntax: .item \r\nAllows you to assign an item to a vendor!');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('itemmove', 4, 'Syntax: .itemmove\r\nNOT WORKING');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('addmove', 4, 'Syntax: .move\r\nAdd your current location for move.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('random', 4, 'Syntax: .random #\r\nSet random movement! 1=ranom(default), 0=path');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('run', 4, 'Syntax: .run #\r\nSet run or walk! 1=run, 0=walk(default)');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('anim', 4, '');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('animfreq', 4, '');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('commands', 4, '.commands displays GM commands.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('die', 4, 'Kills your character.\r\n');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('dismount', 0, 'Dismount you from a mount');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('displayid', 4, '.displayid changes the skin ID of a NPC');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('factionid', 4, '.factionid changes the faction ID of a NPC');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('gmlist', 0, 'Syntax: .gmlist\r\nLista all online gm''s');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('gmoff', 4, '.gmoff togges <GM> showing infrot of your nickname off.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('gmon', 4, '.gmon togges <GM> showing infrot of your nickname on.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('info', 0, '');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('morph', 4, '.morph changes your skin.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('go', 4, '.go teleports you with coordinits X Y Z');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('npcflag', 4, '.npcflag , changes the flag of the selected NPC.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('security', 4, '');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('worldport', 4, '.worldport , teleports you around the world without the loading screen.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('update', 4, '');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('addgrave', 4, 'add graveyard location to database');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('addsh', 4, '.addsh ,spawns a spirit healrs to you current location. You wont see it.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('npcinfo', 4, '');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('demorph', 4, '.demorph changes you back to your normal skin.\r\n');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('revive', 4, '.revive select a character to revive him.');
INSERT INTO `commands` (`name`, `security`, `help`) VALUES ('addspw', 4, 'Syntax: .addspw <entry id>\r\nAllows to spawn a creature from a creature template using the given template id.\r\n<entry id> = decimal template id\r\n');
