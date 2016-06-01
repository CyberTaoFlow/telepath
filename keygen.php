<?php

        function gen_seed() {

                $seed_min = base_convert ("1111", 36, 10);
                $seed_max = base_convert ("ZZZZ", 36, 10);
                $seed_1   = base_convert (mt_rand($seed_min, $seed_max), 10, 36);
                $seed_2   = base_convert (mt_rand($seed_min, $seed_max), 10, 36);

                return strtoupper($seed_1 . $seed_2);

        }

        function gen_timestamp() {

                $epoch     = strtotime("1 January 2014 00:00:00");
                $now       = time();
                $hours     = intval(($now - $epoch) / 3600);
				
				if(isset($_REQUEST['target']) && $_REQUEST['target'] != '') {
					$hours = intval(((strtotime($_REQUEST['target']) - $epoch) - (30 * 24 * 3600)) / 3600);
					echo $_REQUEST['target'] . '<br>';
				}
				
				// echo 'Hours since epoch ' . $hours . "\n";
				// Custom hours::
                //$hours = 15650;
				
                $b32_hours = base_convert($hours, 10, 36);

                // Limited to 2020
                while(strlen($b32_hours) < 3) {
                        $b32_hours = '0' . $b32_hours;
                }

                // Extra obfuscation bit
                $b32_hours .= base_convert (mt_rand(0, 35), 10, 36);

                return strtoupper($b32_hours);

        }
		
        function gen_sign($seed, $timestamp) {

                // Salt values
                $salt_time = 'HybridTime~!';
                $salt_seed = 'HybridSeed~!';

                // Seed signature
                $sign_seed = strtoupper(substr(sha1($seed . $salt_seed), 0, 12));
                // Time signature
                $sign_time = strtoupper(substr(sha1($timestamp . $salt_time), 0, 4));

                $key = $seed . '-' . $timestamp . '-' . $sign_time . '-' . 'TPTH' . '-' . $sign_seed;

                return $key;
        }


        function gen_validate($key) {

                // Salt values
                $salt_time = 'HybridTime~!';
                $salt_seed = 'HybridSeed~!';

                // Length = 36
                if(strlen($key) != 36) { return false; }
                // Portions = 5
                $key = explode('-', $key);
                if(count($key) != 5) { return false; }
                if(strlen($key[0]) != 8)  { return false; } // 8 bits SEED
                if(strlen($key[1]) != 4)  { return false; } // 4 bits TIME
                if(strlen($key[2]) != 4)  { return false; } // 4 bits SIGN_TIME
                if(strlen($key[3]) != 4)  { return false; } // 4 bits PRODUCT
                if(strlen($key[4]) != 12) { return false; } // 12 bits SIGN_SEED

                // SIGN_SEED = UPPER SHA1(SEED + SALT_SEED) LIMIT 12
                $sign_seed = strtoupper(substr(sha1($key[0] . $salt_seed), 0, 12));

                // SIGN_SEED = UPPER SHA1(TIME + SALT_TIME) LIMIT 4
                $sign_time = strtoupper(substr(sha1($key[1] . $salt_time), 0, 4));

                if($key[2] != $sign_time) { return false; }
                if($key[3] != 'TPTH')     { return false; }
                if($key[4] != $sign_seed) { return false; }

                $epoch_diff = base_convert(substr($key[1], 0, 3), 36, 10) * 3600;
                $epoch = strtotime("1 January 2014 00:00:00");

                return $epoch + $epoch_diff;

        }


        function gen_gen() {

                $seed    = gen_seed();
                $stamp   = gen_timestamp();
                $key     = gen_sign($seed, $stamp);
                return $key;

        }

	$key = gen_gen();
	echo 'Key :: ' . $key . "\n"  . "<br>";
	echo 'Is valid until :: ' . date('d/m/Y', strtotime('+30 days', gen_validate($key)));
	

?>
<br><br>
<form>
  Target expiration:
  <input type="date" name="target">
  <input type="submit">
</form>

