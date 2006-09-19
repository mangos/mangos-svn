-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:08 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `areatriggers`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `areatriggers`;
CREATE TABLE IF NOT EXISTS `areatriggers` (
  `id` bigint(20) NOT NULL auto_increment,
  `script` text NOT NULL,
  `name` text NOT NULL,
  `note` text NOT NULL,
  `mapid` int(10) default '0',
  `coord_x` float default '0',
  `coord_y` float default '0',
  `coord_z` float default '0',
  `totrigger` int(10) default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) TYPE=MyISAM COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=1 ;
