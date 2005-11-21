-- phpMyAdmin SQL Dump
-- version 2.6.4-pl3
-- http://www.phpmyadmin.net
-- -- Host: localhost
-- Generation Time: Oct 31, 2005 at 10:28 AM
-- Server version: 4.1.14
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- 
---------------------------------------------------------- 
-- Table structure for table `gmtickets`
-- 
-- Creation: Oct 31, 2005 at 10:28 AM
-- Last update: Oct 31, 2005 at 10:28 AM
-- 
DROP TABLE IF EXISTS `gmtickets`;
CREATE TABLE `gmtickets` (
  `ticket_id` int(11) NOT NULL auto_increment,
  `guid` int(6) unsigned NOT NULL default '0',
  `ticket_text` varchar(255) NOT NULL default '',
  `ticket_category` int(1) NOT NULL default '0',
  PRIMARY KEY  (`ticket_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
  