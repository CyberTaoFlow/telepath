#!/usr/bin/python
import os
import sys
import urlparse
import time
import string
from dateutil import parser
import socket
import struct
import signal
import xml.dom.minidom
import lxml.html as lh
import redis
import msgpack
import json
import hashlib
import sqlite3 as lite

#con = lite.connect('suricata.db')
#cur = con.cursor()
#cur.execute("CREATE TABLE IF NOT EXISTS Q(R BLOB)")

import mysql.connector
from mysql.connector import errorcode

from multiprocessing import Process, Value, Lock
from Queue import Queue
from threading import Thread

stop = False

r = redis.StrictRedis(host='localhost', port=6379, db=0)

from ctypes import c_int
import fileinput

login_hashes = {}

# Dropped Ignore
drop_ign = Value(c_int)
drop_ign_lock = Lock()
def inc_drop_ign():
	with drop_ign_lock:
		drop_ign.value += 1

# Dropped Extension
drop_ext = Value(c_int)
drop_ext_lock = Lock()
def inc_drop_ext():
	with drop_ext_lock:
		drop_ext.value += 1		

# Dropped Host
drop_host = Value(c_int)
drop_host_lock = Lock()
def inc_drop_host():
	with drop_host_lock:
		drop_host.value += 1

# Dropped Path
drop_path = Value(c_int)
drop_path_lock = Lock()
def inc_drop_path():
    with drop_path_lock:
        drop_path.value += 1
	
# Dropped Weird - Request Line
drop_weird_rql = Value(c_int)
drop_weird_rql_lock = Lock()
def inc_drop_weird_rql():
    with drop_weird_rql_lock:
        drop_weird_rql.value += 1

# Dropped Weird - Request Headers
drop_weird_rqh = Value(c_int)
drop_weird_rqh_lock = Lock()
def inc_drop_weird_rqh():
    with drop_weird_rqh_lock:
        drop_weird_rqh.value += 1
		
# Dropped Weird - Response Line
drop_weird_rsl = Value(c_int)
drop_weird_rsl_lock = Lock()
def inc_drop_weird_rsl():
    with drop_weird_rsl_lock:
        drop_weird_rsl.value += 1
		
# Dropped Weird - Response Headers
drop_weird_rsh = Value(c_int)
drop_weird_rsh_lock = Lock()
def inc_drop_weird_rsh():
    with drop_weird_rsh_lock:
        drop_weird_rsh.value += 1
		
# Dropped JSON
drop_json = Value(c_int)
drop_json_lock = Lock()
def inc_drop_json():
    with drop_json_lock:
        drop_json.value += 1

# Stats Pass
stat_pass = Value(c_int)
stat_pass_lock = Lock()
def inc_stat_pass():
    with stat_pass_lock:
        stat_pass.value += 1
		
def work( data ):
	
	stop = False	
	output = { 
		
		# Request
		"TT": "", # Request time
		"TI": "", # Request IP
		"TM": "", # Request method
		"TU": "", # Request URI
		"TB": "", # Request body
		# TH_<key> -- Headers
		# TG_<key> -- GET Param
				
		# Response
		"RI": "",  # Response IP
		"RC": "",  # Response Code
		"RB": "",  # Response Body
		# RH_<key> -- Headers
		
	}
		
	#try:
	
	if not 'rql' in data:
		#r.lpush('errors', json.dumps(data))
		inc_drop_weird_rql()
		return
		
	if not 'rqh' in data:
		#r.lpush('errors', json.dumps(data))
		inc_drop_weird_rqh()
		return
	
	if not 'rsl' in data:
		#print data
		#r.lpush('errors', json.dumps(data))
		inc_drop_weird_rsl()
		return
		
	if not 'rsh' in data:
		#r.lpush('errors', json.dumps(data))
		inc_drop_weird_rsh()
		return
	
	#except:
	#	pass
	
	try:
	
		# REQUEST HEADERS
		stop = False
		tmp = data['rqh'].strip().split("\n")
		for line in tmp:
		
			line = line.split(": ", 2)
			if len(line) == 2:
				
				line[0] = line[0].strip()
				line[1] = line[1].strip()
				
				if line[0].lower() == "host":
					if False:
						inc_drop_host()
						stop = True
					else:
						line[0] = "Host"
			
			output["TH_" + line[0]] = line[1]
		
		if stop:
			return
		
	except:
		pass
		
	try:
	
		# REQUEST LINE
		tmp = data['rql'].split(' ', 3)
		if len(tmp) > 1:
			output["TM"] = tmp[0]
		else:
			#r.lpush('errors', json.dumps(data))
			inc_drop_weird_rql()
			return
	
	except:
		pass
	
	if len(tmp) > 1:
		
		try:
			tmp = urlparse.urlparse(tmp[1])
		except:
			pass
		
		# PATH
		if tmp.path:
			
			extension = os.path.splitext(tmp.path)[1]
			if extension in drop_ext_list:
				#print "Dropped EXT " + extension
				inc_drop_ext()
				return
			
			output["TU"] = tmp.path

		# QUERY
		if tmp.query:
			tmp = urlparse.parse_qsl(tmp.query, 1)
			for key, item in tmp:
				output["TG_" + key] = item
	
	
	# CHECK IF IP IN IGNORE LIST
	try:
		src_long = struct.unpack("!L", socket.inet_aton(data["src"]))[0]
	except:
		print("Dropped Weird - Bad request IP " + data["src"])
		#inc_drop_weird_ip()
		return
				
	for drop_ign_item in drop_ign_list:
		if src_long >= drop_ign_item[0] and src_long <= drop_ign_item[1]:
			#print("Dropped Ignore - " + data["src"])
			inc_drop_ign()
			stop = True
			break
	
	if stop:
		return
	
	try:
	
		# REQUEST IP
		output["TI"] = data["src"]

		# RESPONSE IP
		output["RI"] = data["dst"]
	
	except:
			pass
		
	try:
		# RESPONSE HEADERS
		tmp = data['rsh'].strip().split("\n")
		for line in tmp:
			line = line.split(":", 2)
			if len(line) == 2:
				output["RH_" + line[0].strip()] = line[1].strip()
	
	except:
	
		#try:
		#	r.lpush('errors', json.dumps(data))
		#except:
		#	pass
			
		inc_drop_weird_rsh()
		return
	
	try:
		#RESPONSE LINE
		tmp = data["rsl"].split(" ", 3)
		if len(tmp) > 2:
			output["RC"] = tmp[1];
	except:
			pass
	
	try:
	
		# REQUEST TIME
		output["TT"] = str(int(time.mktime(parser.parse(data["ts"]).timetuple())))
		
	except:
			pass
	
	# POST / BODY
	if 'rqb' in data:
		output["TB"] = data["rqb"]
		
	# RESPONSE BODY
	if 'rsb' in data:
		output["RB"] = data["rsb"]
			
	try:
	
		data = msgpack.packb(output)
		r.lpush('Q', data)
		inc_stat_pass()
		#binary = lite.Binary(data)
		#cur.execute("INSERT INTO Q(R) VALUES (?)", (binary,) )
		#con.commit()
		
	except:
			pass

def process( queue ):
	
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	finder = Finder()
	
	while not exitapp:
		
		stop = False;
		data = queue.get()
		work (data)
		
			
		
def reload_mysql():

	global drop_ign_list

	f = open('/opt/telepath/conf/atms.conf')
	content = f.readlines()
	for i in content:
		key = i.split("\"",2)[0]
		if(key=="username="):
			l_username = i.split("\"",2)[1]
 		elif(key=="password="):
			l_password = i.split("\"",2)[1]
		elif(key=="database_address="):
			l_db_addr = i.split("\"",2)[1]
		elif(key=="database_name="):
			l_db_name = i.split("\"",2)[1]

        config = {
          'user': l_username,
          'password': l_password,
          'host': l_db_addr,
          'database': l_db_name,
          'raise_on_warnings': True,
        }

	try:
		cnx = mysql.connector.connect(**config)
	except mysql.connector.Error as err:
		if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
			print("Something is wrong with your user name or password")
		elif err.errno == errorcode.ER_BAD_DB_ERROR:
			print("Database does not exists")
		else:
			print(err)
	else:
		
		#print("MySQL CNX")
		
		cursor = cnx.cursor()
		query = ("SELECT user_ip from whitelist_ip")
		cursor.execute(query)
	
		for (user_ip) in cursor:
		
			user_ip = user_ip[0].strip().split("-")
			
			try:
				long_start = struct.unpack("!L", socket.inet_aton(user_ip[0]))[0]
				long_end   = long_start
				
				if len(user_ip) > 1:
					long_end = struct.unpack("!L", socket.inet_aton(user_ip[1]))[0]
				
			except:
				print("Malformed IP")
				
			drop_ign_list.append((long_start, long_end))
		
		#print("MySQL CLOSE")
		cursor.close()
		cnx.close()
		
		pass
		
		
		

# MAIN PROGRAM STARTS HERE

drop_ext_list = ['.css', '.pac', '.js', '.jpg', '.jpeg', '.png', '.bmp', '.pdf']
drop_ign_list = []
stat_total = 0
stat_sec   = 0
stat_tick  = int(time.time())
exitapp = False
reload_mysql()
reload_tick  = int(time.time())
reload_interval = 600


#queue = Queue()
#num_threads = 20
#for i in range(num_threads):
#	worker = Thread(target=process, args=(queue,))
#	worker.setDaemon(True)
#	worker.start()

zero_data = 0

while not exitapp:
	
	line = sys.stdin.readline()
	stat_total = stat_total + 1
	stat_sec   = stat_sec + 1
	data = False	

	try:
		line = unicode(line, errors='ignore')
		data = json.loads( line )
	#except KeyboardInterrupt:
	#	exitapp = True
	#	raise
	except:
		print("[suricata.py] No More Data To Read")
		time.sleep(0.1)
		exit()
	
	if data == False:
		#print line
		exit()
		inc_drop_json()
		time.sleep(0.1)
	else:
		work(data)
	
	now = int(time.time())
	
	if now > reload_tick + reload_interval:
		reload_mysql()
		reload_tick = now

	if now > stat_tick:
		stat_tick = now
		stats = "RQS1 " + str(stat_sec)  + " TOTAL " + str(stat_total) + " IGN " + str(drop_ign.value) + " EXT " + str(drop_ext.value) + " HOST " + str(drop_host.value) + " WEIRD RQL " + str(drop_weird_rql.value) + " WEIRD RQH " + str(drop_weird_rqh.value) + " WEIRD RSL " + str(drop_weird_rsl.value) + " WEIRD RSH " + str(drop_weird_rsh.value) + " PASS " + str(stat_pass.value) + " NOT JSON " + str(drop_json.value)
		#print stats
		r.set('stats', stats)		
		stat_sec = 0

print("Python EOF")
