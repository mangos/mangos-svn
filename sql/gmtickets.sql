CREATE TABLE gmtickets (
  ticket_id int(11) NOT NULL auto_increment,
  guid int(6) unsigned NOT NULL default '0',
  ticket_text varchar(255) NOT NULL default '',
  ticket_category int(1) NOT NULL default '0',
  PRIMARY KEY  (ticket_id)
) TYPE=MyISAM;
