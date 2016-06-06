CREATE TABLE IF NOT EXISTS `ci_perm_data` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `class` varchar(64) NOT NULL,
  `function` varchar(64) NOT NULL,
  `alias` varchar(128) NOT NULL,
  `description` varchar(512) NOT NULL,
  `params` varchar(1024) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=20 ;