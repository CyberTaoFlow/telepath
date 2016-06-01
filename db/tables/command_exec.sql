CREATE TABLE IF NOT EXISTS `command_exec` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(32) NOT NULL,
  `cmd` text NOT NULL,
  `src_ip` tinyint(1) NOT NULL DEFAULT '0',
  `resp_ip` tinyint(1) NOT NULL DEFAULT '0',
  `timestamp` tinyint(1) NOT NULL DEFAULT '0',
  `rule_group` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=3 ;