#!/usr/bin/python

import sys
import redis
r = redis.StrictRedis(host='localhost', port=6379)

from elasticsearch import Elasticsearch
es = Elasticsearch()

key = "_id"
val = sys.argv[1]

res = es.search( body = { "query": { "term": { key : val } } } )

with open("test.txt", "a") as myfile:
            myfile.write(sys.argv[1] + "\n")

if len(res['hits']['hits']) == 1:
	req = res['hits']['hits'][0]['_source']
	r.setex('CAPTCHA_' + req['ip_orig'], 600, '')

