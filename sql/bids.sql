-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:14 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `bids`
-- 
-- Creation: Aug 21, 2005 at 10:31 AM
-- Last update: Aug 21, 2005 at 10:31 AM
-- 

DROP TABLE IF EXISTS `bids`;
CREATE TABLE `bids` (
  `bidder` int(32) NOT NULL default '0',
  `ID` int(32) NOT NULL default '0',
  `amt` int(32) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
