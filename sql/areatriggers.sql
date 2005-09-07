-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:14 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `areatriggers`
-- 
-- Creation: Aug 22, 2005 at 04:31 PM
-- Last update: Aug 22, 2005 at 04:31 PM
-- 

DROP TABLE IF EXISTS `areatriggers`;
CREATE TABLE `areatriggers` (
  `id` bigint(20) NOT NULL auto_increment,
  `script` text NOT NULL,
  `name` text NOT NULL,
  `mapid` int(10) default '0',
  `coord_x` float default '0',
  `coord_y` float default '0',
  `coord_z` float default '0',
  `totrigger` float default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=3710 ;
