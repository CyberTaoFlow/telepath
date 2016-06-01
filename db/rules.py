from elasticsearch import Elasticsearch
from os import listdir
import json

json_output = '/opt/telepath/db/elasticsearch/rules.json'
#files = listdir('rules/')

def get_elastic_rules():
	elastic_rules_list = []
	es = Elasticsearch()
	es.delete_by_query(index='telepath-rules', body={"query":{"bool":{"must":[{"query_string":{"default_field":"rules.owner","query":"hybrid"}}],"must_not":[],"should":[]}}})        
	return elastic_rules_list

def get_file_rules():
	file_rules_list = []
	with open(json_output, 'r') as rules:
		lines = rules.readlines()
		for line in lines:
			rule_dict = json.loads(line)
			rule_name = repr(rule_dict['name'])
			file_rules_list.append(rule_name)
		return file_rules_list

def add_new_rule(name):
	literal_name = name.replace("u'", "").replace("'","")
	with open(json_output, 'r') as rules:
		lines = rules.readlines()
		for line in lines:
			rule_dict = json.loads(line)
			rule_name = repr(rule_dict['name'])
			lit = rule_name.replace("u'", "").replace("'","")
			if lit == literal_name:
				es = Elasticsearch()
				insert = es.index(index="telepath-rules", doc_type='rules', body=line)
				print('New rule added!')

def compare_rules():
	print('Checking for changes in rules source...')
	el = get_elastic_rules()
	fl = get_file_rules()
	total_exists = 0
	total_not_exists = 0
	for fr in fl:
		if fr in el:
			total_exists = total_exists + 1
		else:
			add_new_rule(fr)
			total_not_exists = total_not_exists + 1
	if total_not_exists == 0:
		print('No new rules found.')
	else:
		print('ADDED {} NEW RULE(S)'.format(total_not_exists))

if __name__ == '__main__': 
	
	es = Elasticsearch()
        es.indices.create(index='telepath-rules', ignore=400)
	compare_rules()
