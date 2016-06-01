CREATE TABLE IF NOT EXISTS `subdomain_ips` (
  `subdomain_id` int(10) unsigned NOT NULL,
  `resp_ip` varchar(20) NOT NULL,
  UNIQUE KEY `resp_ip` (`subdomain_id`,`resp_ip`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;