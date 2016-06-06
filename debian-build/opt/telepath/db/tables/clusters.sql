CREATE TABLE IF NOT EXISTS `clusters` (
		  `id` int(11) NOT NULL AUTO_INCREMENT,
		  `hostname` int(10) unsigned NOT NULL,
		  `x_centoid` double NOT NULL,
		  `y_centoid` double NOT NULL,
		  `radius` double NOT NULL,
		  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1;