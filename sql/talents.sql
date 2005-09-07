-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:19 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `talents`
-- 
-- Creation: Aug 26, 2005 at 12:26 AM
-- Last update: Aug 26, 2005 at 12:26 AM
-- 

DROP TABLE IF EXISTS `talents`;
CREATE TABLE `talents` (
  `id` int(10) NOT NULL auto_increment,
  `t_id` int(10) NOT NULL default '0',
  `maxrank` int(7) NOT NULL default '0',
  `class` int(10) NOT NULL default '0',
  `rank1` longtext collate latin1_general_ci NOT NULL,
  `rank2` longtext collate latin1_general_ci NOT NULL,
  `rank3` longtext collate latin1_general_ci NOT NULL,
  `rank4` longtext collate latin1_general_ci NOT NULL,
  `rank5` longtext collate latin1_general_ci NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=1 ;
