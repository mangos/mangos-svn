DROP TABLE IF EXISTS `spell_script_target`;
CREATE TABLE `spell_script_target` (
  `entry` int(6) unsigned NOT NULL,
  `type` int(8) unsigned default '0',
  `targetEntry` int(11) default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Spell System';

INSERT INTO `spell_script_target` VALUES 
/* CREATURE */
(11637, 1, 6220),
(11637, 1, 6218),
(11637, 1, 6219),
(12709, 1, 6219),
(12709, 1, 6220),
(12709, 1, 6218),
/* DEAD */
(31210, 2, 17544),
(31225, 2, 17768),
/* GO */
(33067, 0, 0), 
(39094, 0, 0);
