-- 
-- Dumping data for table `gameobject_template`
-- 

DELETE FROM `gameobject_template` WHERE `entry` IN (180058, 180059, 180060, 180061, 180087, 180088, 180089, 180090, 180091, 180255, 180256, 180100, 180101, 180102);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `faction`, `flags`, `size`, `sound0`, `sound1`, `sound2`, `sound3`, `sound4`, `sound5`, `sound6`, `sound7`, `sound8`, `sound9`, `sound10`, `sound11`, `sound12`, `sound13`, `sound14`, `sound15`, `sound16`, `sound17`, `sound18`, `sound19`, `sound20`, `sound21`, `sound22`, `sound23`, `ScriptName`) VALUES 
(180058, 1, 6251, 'Alliance Banner', 83, 32, 1, 0, 1479, 196608, 180100, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180059, 1, 6252, 'Contested Banner', 83, 32, 1, 0, 1479, 0, 180102, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180060, 1, 6253, 'Horde Banner', 84, 32, 1, 0, 1479, 196608, 180101, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180061, 1, 6254, 'Contested Banner', 84, 32, 1, 0, 1479, 0, 180102, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180087, 10, 6271, 'Stable Banner', 35, 0, 1, 1479, 0, 0, 196608, 0, 0, 0, 0, 0, 0, 23932, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ''),
(180088, 10, 6271, 'Blacksmith Banner', 35, 0, 1, 1479, 0, 0, 196608, 0, 0, 0, 0, 0, 0, 23936, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ''),
(180089, 10, 6271, 'Farm Banner', 35, 0, 1, 1479, 0, 0, 196608, 0, 0, 0, 0, 0, 0, 23935, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ''),
(180090, 10, 6271, 'Lumber Mill Banner', 35, 0, 1, 1479, 0, 0, 196608, 0, 0, 0, 0, 0, 0, 23938, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ''),
(180091, 10, 6271, 'Mine Banner', 35, 0, 1, 1479, 0, 0, 196608, 0, 0, 0, 0, 0, 0, 23937, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ''),
(180100, 6, 2232, 'Alliance Banner Aura', 114, 32, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180101, 6, 1311, 'Horde Banner Aura', 114, 32, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180102, 6, 266, 'Neutral Banner Aura', 114, 32, 5, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180255, 0, 6353, 'ALLIANCE DOOR', 114, 32, 1.58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(180256, 0, 6354, 'HORDE DOOR', 114, 32, 1.57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '');


-- 
-- Dumping data for table `creature_template`
-- 

DELETE FROM `creature_template` WHERE `entry` IN (13116, 13117);

INSERT INTO `creature_template` (`entry`, `modelid_m`, `modelid_f`, `name`, `subname`, `minlevel`, `maxlevel`, `minhealth`, `maxhealth`, `minmana`, `maxmana`, `armor`, `faction`, `npcflag`, `speed`, `rank`, `mindmg`, `maxdmg`, `attackpower`, `baseattacktime`, `rangeattacktime`, `flags`, `dynamicflags`, `size`, `family`, `bounding_radius`, `trainer_type`, `trainer_spell`, `class`, `race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `combat_reach`, `type`, `civilian`, `flag1`, `equipmodel1`, `equipmodel2`, `equipmodel3`, `equipinfo1`, `equipinfo2`, `equipinfo3`, `equipslot1`, `equipslot2`, `equipslot3`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `RacialLeader`, `ScriptName`) VALUES 
(13116, 13336, 0, 'Alliance Spirit Guide', NULL, 60, 60, 8609, 8609, 7860, 7860, 0, 4, 65, 1.76, 1, 134, 280, 1449, 1175, 1292, 0, 0, 1, 0, 2, 0, 0, 0, 0, 62.016, 85.272, 100, 3.65, 7, 1, 2, 22802, 0, 0, 285346306, 0, 0, 2, 0, 0, 13116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 844, '', 0, 3, 0, ''),
(13117, 13338, 0, 'Horde Spirit Guide', NULL, 60, 60, 8609, 8609, 7860, 7860, 0, 2, 65, 1.76, 1, 134, 280, 1449, 1175, 1292, 0, 0, 1, 0, 2, 0, 0, 0, 0, 62.016, 85.272, 100, 3.65, 7, 1, 2, 24015, 0, 0, 285346306, 0, 0, 1, 0, 0, 13117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 844, '', 0, 3, 0, '');

