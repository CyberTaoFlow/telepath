from idstools import unified2
import re
import json
import string
import random
import memcache


mc = memcache.Client(['127.0.0.1:11211'], debug=1)

reader = unified2.SpoolEventReader("/opt/telepath/suricata/logs/","unified2.alert", follow = True)

rules = {}
with open("/opt/telepath/suricata/suricata.rules") as f:
	content = f.readlines()

for line in content:
	result = re.match(r'.*msg:\"([^\"]+)\".*sid:([^;]+)', line)
	if result:
		rules[result.groups()[1]] = result.groups()[0]

#print rules

for record in reader:
	
	if 'signature-id' in record and 'destination-ip' in record and 'source-ip' in record and 'event-second' in record:

		out = { 
			"ts": record['event-second'], 
			"rule": rules[str(record['signature-id'])], 
			"src": record['source-ip'], 
			"dst": record['destination-ip'], "data": '' 
		}
		
		if 'packets' in record:
			for packet in record['packets']:
				filtered_data = filter(lambda x: x in string.printable, packet['data'])
				out['data'] = out['data'] + filtered_data
		
		print json.dumps(out)
		key = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(10))
		mc.set("alert_" + key, json.dumps(out))

