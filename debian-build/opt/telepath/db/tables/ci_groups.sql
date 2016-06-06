CREATE TABLE IF NOT EXISTS `ci_groups` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `description` varchar(512) NOT NULL,
  `extradata` varchar(4048) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=3;
