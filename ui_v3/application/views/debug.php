<?php
defined('BASEPATH') OR exit('No direct script access allowed');

$css = '.flag { clear: both; float: left; margin-right: 10px; display: block; height: 20px; width: 20px; background: transparent url("/v3/img/flags.png") 0 0 no-repeat; }';

$order = array ( 
	// Line #1
	0 => 'ad ', 1 => 'ae ', 2 => 'af ', 3 => 'al ', 4 => 'am ', 5 => 'ao ', 6 => 'ar ', 7 => 'at ', 8 => 'au ', 9 => 'az ', 10 => 'ba ', 11 => 'bb ', 12 => 'bd ', 13 => 'be ', 14 => 'bf ', 15 => 'bg ', 16 => 'bh ', 17 => 'bi', 18 => 'bj ', 19 => 'bn ', 20 => 'bo ', 21 => 'br ', 22 => 'bs ', 23 => 'bt ', 24 => 'bw ', 25 => 'by ', 
	// Line #2 
	26 => 'az ', 27 => 'ca ', 28 => 'cd ', 29 => 'cf ', 30 => 'cg ', 31 => 'ch ', 32 => 'ci ', 33 => 'cl ', 34 => 'cm ', 35 => 'cn ', 36 => 'co ', 37 => 'cr ', 38 => 'cu ', 39 => 'cv ', 40 => 'cy ', 41 => 'cz ', 42 => 'de ', 43 => 'dj ', 44 => 'dk ', 45 => 'dm ', 46 => 'do ', 47 => 'dz ', 48 => 'ec ', 49 => 'ee ', 50 => 'eg ', 51 => 'ef ', 
	// Line #3	
	52 => 'es ', 53 => 'et ', 54 => 'fi ', 55 => 'fj ', 56 => 'fm ', 57 => 'fr ', 58 => 'ga ', 59 => 'gd ', 60 => 'ge ', 61 => 'gh ', 62 => 'gm ', 63 => 'gn ', 64 => 'gq ', 65 => 'gr ', 66 => 'gt ', 67 => 'gw ', 68 => 'hh ', 69 => 'hr ', 70 => 'ht ', 71 => 'hu ', 72 => 'id ', 73 => 'ie ', 74 => 'il ', 75 => 'in ', 76 => 'ig ', 77 => 'ir ', 
	// Line #4
	78 => 'is ', 79 => 'it ', 80 => 'jm ', 81 => 'jo ', 82 => 'jp ', 83 => 'ke ', 84 => 'kg ', 85 => 'kh ', 86 => 'ki ', 87 => 'km ', 88 => 'kn ', 89 => 'kp ', 90 => 'kr ', 91 => 'kw ', 92 => 'kz ', 93 => 'la ', 94 => 'lb ', 95 => 'li ', 96 => 'lk ', 97 => 'lr ', 98 => 'ls ', 99 => 'lt ', 100 => 'lu ', 101 => 'lv ', 102 => 'ly ', 103 => 'ma ', 
	// Line #5
	104 => 'mc ', 105 => 'md ', 106 => 'me ', 107 => 'mg ', 108 => 'mh ', 109 => 'mk ', 110 => 'ml ', 111 => 'mm ', 112 => 'mn ', 113 => 'mr ', 114 => 'mt ', 115 => 'mu ', 116 => 'mw ', 117 => 'mx ', 118 => 'my ', 119 => 'mz ', 120 => 'na ', 121 => 'ne ', 122 => 'ng ', 123 => 'ni ', 124 => 'nl ', 125 => 'no ', 126 => 'np ', 127 => 'nr ', 128 => 'nz ', 129 => 'om ', 
	// Line #6
	130 => 'pa ', 131 => 'pe ', 132 => 'pg ', 133 => 'ph ', 134 => 'pk ', 135 => 'pl ', 136 => 'pt ', 137 => 'pw ', 138 => 'py ', 139 => 'qa ', 140 => 'ro ', 141 => 'rs ', 142 => 'ru ', 143 => 'rw ', 144 => 'sa ', 145 => 'sb ', 146 => 'sc ', 147 => 'sd ', 148 => 'se ', 149 => 'sg ', 150 => 'si ', 151 => 'sk ', 152 => 'sl ', 153 => 'sm ', 154 => 'sn ', 155 => 'so ', 
	// Line #7
	156 => 'sr ', 157 => 'ss ', 158 => 'st ', 159 => 'sv ', 160 => 'sy ', 161 => 'sz ', 162 => 'td ', 163 => 'tg ', 164 => 'th ', 165 => 'tj ',	166 => 'tm ', 167 => 'tn ', 168 => 'to ', 169 => 'tp ', 170 => 'tr ', 171 => 'tt ', 172 => 'tv ', 173 => 'tw ', 174 => 'tz ', 175 => 'ua ', 176 => 'ug ',177 => 'uk ', 178 => 'us ', 179 => 'uy ', 180 => 'uz ', 181 => 'va ', 
	// Line #8
	182 => 'vc ', 183 => 'vd ', 184 => 'vn ', 185 => 've ', 186 => 'vu ', 187 => 'wl ', 188 => 'ws ', 189 => 'ye ', 190 => 'za ', 191 => 'zm ', 192 => 'zw'
);

$perline = 26;
$i       = 0;

for($i = 0; $i < count($order); $i++) {

	$row = $i % $perline;
	$order[$i] = trim(strtoupper($order[$i]));
	$bg_pos = (-1 * ($row * 20)) . 'px ' . (-1 * (intval($i / $perline) * 40)) . 'px';
	$css   .= '.flag.flag-' . $order[$i] . ' { background-position: ' . $bg_pos . '; }' . "\n";
	
}

echo $css;

?>