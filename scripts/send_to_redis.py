#!/usr/bin/python
# coding=utf-8
import time
from tqdm import *
import redis
import json
import random
import string
from faker import Factory
from random import randint, randrange
from datetime import datetime
from elasticsearch import Elasticsearch

es = Elasticsearch()

fake = Factory.create()

rd = redis.Redis(host='localhost', port=6379, db=0)

def gen_string(string_range):
    gen = ''.join([random.choice(string.ascii_lowercase) for n in xrange(string_range)])
    return gen

def gen_randint():
    gen = random.randint(1,255)
    return gen

def gen_IP():
    ip = "{}.{}.{}.{}".format(gen_randint(), gen_randint(), gen_randint(), gen_randint())
    return str(ip)

def gen_URL():
    url = "{}/{}/{}/".format(gen_string(18), gen_string(18), gen_string(18))
    return str(url)

def gen_Cookie():
    ci_session = "ci_session=qa_{}".format(gen_string(12))
    session_id = "session_id=qa_{}".format(gen_string(12))
    ip_address = "ip_address=qa_{}".format(gen_string(12))
    user_agent = "user_agent=qa_{}".format(gen_string(12))
    last_activity = "last_activity=qa_{}".format(gen_string(12))
    user_data = "user_data=qa_{}".format(gen_string(12))
    identity = "identity=qa_{}".format(gen_string(12))
    admin1 = "admin=qa_{}".format(gen_string(12))
    username = "username=qa_{}".format(gen_string(12))
    admin2 = "admin=qa_{}".format(gen_string(12))
    email = "email=qa_{}".format(gen_string(12))
    user_id = "user_id=qa_{}".format(gen_string(12))
    old_last_login = "old_last_login=qa_{}".format(gen_string(12))
    cookie = ci_session + session_id + ip_address + user_agent + last_activity + user_data + identity + admin1 + username + admin2 + email + user_id + old_last_login 
    return cookie
    
def gen_res_code():
    response_codes = ["200","302","404","403","500"]
    random_index = randrange(0,len(response_codes))
    return str(response_codes[random_index])

def gen_post():
    post = "arabic=مرحبا بالعالم&armenian=բարեւ աշխարհը&bengali=হ্যালো বিশ্ব&chinese=你好世界&greek=αυτό είναι ένα απλό φράση&gujarati=આ એક સરળ શબ્દસમૂહ છે&hebrew=זה ביטוי פשוט&hindi=यह एक साधारण वाक्य है&icelandic=þetta er einfalt setning&japanese=これは、単純なフレーズである&kannada=ಈ ಸರಳ ನುಡಿಗಟ್ಟು ಆಗಿದೆ&khmer=នេះគឺជាឃ្លាដ៏សាមញ្ញ&korean=이 간단한 문구&lao=ນີ້ແມ່ນເປັນປະໂຫຍກທີ່ງ່າຍດາຍ&nepali=यो एक सरल वाक्यांश छ&romanian=aceasta este o frază simplă&russian=это простая фраза&tamil=இந்த ஒரு எளிய சொற்றொடர்&telugu=ఈ ఒక సాధారణ పదబంధం ఉంది&thai=นี้เป็นวลีที่ง่าย&vietnamese=đây là một cụm từ đơn giản&yiddish=דעם איז אַ פּשוט פראַזע&yorubayi ni a o rọrun gbolohun"

    selected_apps = "selected_apps=qa_hybrid_{}".format(gen_string(5))
    scale = "scale=qa_hybrid_{}+{}&".format(gen_string(5), gen_string(5))
    items = "items=qa_hybrid_{}&".format(gen_string(5))
    gap = "gap=qa_hybrid_{}".format(gen_string(5))
    post = selected_apps + scale + items + gap
    return post

def gen_keep_alive():
    keep_alive = "timeout={}, max={}".format(random.randint(1,100),random.randint(101,200))
    return keep_alive

def gen_output():

    headers_request = {"Content-Length": gen_randint(), 
                       "Accept-Language": "qps-ploc,en-US;q=0.5", 
                       "Accept-Encoding": "gzip, deflate", 
                       "Connection": "keep-alive",
                       "Accept": "application/json, text/javascript, */*; q=0.01", 
                       "User-Agent": fake.user_agent(), 
                       "Host": "192.168.1.111", 
                       "X-Requested-With": 'XMLHttpRequest', 
                       "Cookie": gen_Cookie(),
                       "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
                       "Keep-Alive": gen_keep_alive(),
                       "Accept-Ranges":"bytes"
                       }
    
    headers_response = {"Content-Length": random.randint(1,255),
                        "X-Powered-By": "PHP/5.5.9-1ubuntu4.5",
                        "Keep-Alive": "timeout=5, max=100",
                        "Server": "Apache/2.4.7 (Ubuntu)",
                        "Connection": "Keep-Alive",
                        "Content-Type": "application/json; charset=utf-8",
                        "Keep-Alive": gen_keep_alive(),
                        "Accept-Ranges":"bytes"
                        }
    
    request = {"sip":gen_IP(),
               "url": gen_URL(),
               "ip": "",
               "uri": "",
               "headers": headers_request, 
               "timestamp": datetime.now().strftime('%s'), 
               "query": {},
               "post": gen_post(), 
               "method": "POST"}
    
    response =  {"headers": headers_response,
                 "code": gen_res_code(),
                 "dip": gen_IP()}
    
    output = {'request':request,
              'response':response}
    return output

def get_hits():
    index = "telepath-{}".format(time.strftime("%Y%m%d"))
    query = es.search(index=index, body={"query": {"match_all": {}}})
    total_hits = query['hits']['total']
    es.indices.refresh(index=index)
    return total_hits

def run():
    user_flush = raw_input('Flush Redis data before sending requests? (y/N):')
    if user_flush == 'Y' or user_flush == 'y':
      conf = raw_input('This action will DELETE ALL REDIS DATA. Proceed? (y/N):')
      if conf == 'Y' or conf == 'y':
        rd.flushall()
        print('ALL REDIS DATA DELETED!')
        user_range = input('How many requests do you want to send to Redis?')
        user_range_int = int(user_range)
        output = gen_output()
        for req in tqdm(range(user_range_int)):
          rd.lpush('requests', json.dumps(output))
        print('Sent {} requests to Redis!'.format(user_range_int))
      else:
        print('Test canceled!')
    else:
      user_range = input('How many requests do you want to send to Redis?')
      user_range_int = int(user_range)
      output = gen_output()
      for req in tqdm(range(user_range_int)):
        rd.lpush('requests', json.dumps(output))
      print('Sent {} requests to Redis!'.format(user_range_int))

if __name__ == '__main__':
    run()
