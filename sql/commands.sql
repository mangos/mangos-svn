-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:15 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `commands`
-- 
-- Creation: Aug 21, 2005 at 10:32 AM
-- Last update: Aug 21, 2005 at 10:33 AM
-- 

DROP TABLE IF EXISTS `commands`;
CREATE TABLE `commands` (
  `name` varchar(100) collate latin1_general_ci NOT NULL default '',
  `security` int(11) NOT NULL default '0',
  `help` longtext collate latin1_general_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
