CREATE TABLE IF NOT EXISTS `agents_regex` (
  `agent_id` varchar(32),
  `header_name` varchar(64),
  `regex` varchar(1024),
  `cond` tinyint(1),
  `att_id` varchar(32),
  UNIQUE KEY `header_name` (`header_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
