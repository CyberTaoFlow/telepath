CREATE TABLE IF NOT EXISTS `subdomains` (
	`subdomain_id` int(10) unsigned NOT NULL,
	`app_id` int(10) unsigned NOT NULL,
	`subdomain_name` varchar(100) NOT NULL,
	PRIMARY KEY (`subdomain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;