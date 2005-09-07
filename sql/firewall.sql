-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:17 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `firewall`
-- 
-- Creation: Aug 26, 2005 at 12:02 AM
-- Last update: Aug 26, 2005 at 12:02 AM
-- 

DROP TABLE IF EXISTS `firewall`;
CREATE TABLE `firewall` (
  `ip` char(20) collate latin1_general_ci NOT NULL default '',
  `allow` smallint(2) NOT NULL default '0',
  PRIMARY KEY  (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
