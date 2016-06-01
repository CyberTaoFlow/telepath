#!/usr/bin/python
# coding=utf-8
import json
from elasticsearch import Elasticsearch
import urllib2

es = Elasticsearch()

def check():
    url = 'http://127.0.0.1:9200/'
    response = urllib2.urlopen(url)
    query = json.load(response)
    code = query['status']
    return code

def clear():
    res_code = str(check())
    if res_code == '200':
        url = 'http://127.0.0.1:9200/_stats/indices'
        try:
            response = urllib2.urlopen(url)
        except:
            print ("Elasticsearch UNREACHEABLE! CHECK IF ITS ON...")
        query = json.load(response)
        for index in query['indices']:
            if index != 'telepath-config':
                try:
                    es.indices.delete(index=index, ignore=[400, 404])
                except:
                    print ("No indices to delete!")
                print ("Index:{} -- DELETED!".format(index))
    else:
        print('CANNOT ACCESS ELASTICSEARCH. RESPONSE CODE: {}'.format(res_code))

if __name__ == '__main__':
    clear()