-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:20 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `vendors`
-- 
-- Creation: Aug 26, 2005 at 12:29 AM
-- Last update: Aug 26, 2005 at 12:29 AM
-- 

DROP TABLE IF EXISTS `vendors`;
CREATE TABLE `vendors` (
  `vendorGuid` bigint(20) unsigned NOT NULL default '0',
  `itemGuid` bigint(20) unsigned NOT NULL default '0',
  `amount` bigint(20) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 18432 kB';
