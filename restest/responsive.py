#!/usr/bin/python
#script for checking Telepath via responsive platforms
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from os import system
import time
import datetime
import os

# Checking the current SVN Revision and locating it at a local file >>
currentRevistion = system('cd ..; svn info | grep Revision > restest/tmp/rev.txt')

f = open("tmp/rev.txt","r")
fb = open("repold.txt","r")

if f.read() != fb.read():
	# Rewriting the file to the current SVN Revision for next cron updating >>
	f = open("tmp/rev.txt","r")
	fb = open("repold.txt","w")
	text = f.read()
	fb.write(text)
	fb.close()
	f.close()

	baseUrl = "http://192.168.1.111/v3"
	navigationA = ['Dashboard','Cases','Alerts','Suspects']
	navigationC = []
	platforms = [webdriver.Firefox()] # Firefox, Chrome, Ie
	print "working"
	for browser in platforms:
		telepath = browser
		telepath.get(baseUrl)
		waiting = WebDriverWait(telepath, 20)
		cdate = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d')
		ctime = datetime.datetime.fromtimestamp(time.time()).strftime('%H:%M')

		#login to V3 telepath
		telepath.find_element_by_id("login-username").send_keys("admin")
		telepath.find_element_by_id("login-password").send_keys("admin")
		telepath.find_element_by_id("login-button").click()
		time.sleep(2)

		# Minimizing the window for a clean start :)
		actionChain = ActionChains(telepath).key_down(Keys.ALT)
		actionChain.send_keys(Keys.SPACE)
		actionChain.send_keys("n")

		#~~~~Responsive Design For In Loop~~~~:
		# Declaring the required resolutions
		
		screen = [[1366,768], [1024,768], [1920,1080], [1280,800], [1440,900], [1280,1024], [768,1024], [1600,900], [320,568], [300,480]] 
		# Top used Resolutions for all devices: http://gs.statcounter.com/#all-resolution-ww-monthly-201404-201404-bar
		
		path = 'Test '+str(cdate + ' ' + ctime)
		os.mkdir('./screenshots/' + path)
		
		print """
		>> Checking the required Resolutions:
		
		"""
		for i,y in screen:
			ctime = datetime.datetime.fromtimestamp(time.time()).strftime('%H:%M:%S')
			telepath.set_window_size(i,y)
			telepath.refresh()
			resolution = str(i)+'X'+str(y)
			time.sleep(3)
			for page in navigationA:
				telepath.find_element_by_link_text(str(page)).click()
				waiting.until(EC.presence_of_all_elements_located((By.CLASS_NAME, "tele-listitem-inner")))
				time.sleep(12)
				telepath.save_screenshot('./screenshots/' + str(path) + '/' + page + '-' + ctime + ' ' + resolution +'.png')
			print "resolution: " + str(resolution)
		
		# Writing to Log the 
		log = open("log.txt", "a")
		log.write(cdate+" "+ctime+" >> Something has changed \n")
		telepath.close()

	print """
		>>>> Done Testing <<<< 

		>>>> Please Check The New ScreenShots For Errors <<<<

	"""
# else:
# 	# ddate = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d')
# 	# dtime = datetime.datetime.fromtimestamp(time.time()).strftime('%H:%M')
# 	# log = open("log.txt", "a")
# 	# log.write(ddate+" "+dtime+" >> Nothing has changed \n")
# 	print "NOT"
#python -c "from os import system;from re import findall;str(system('svn info | grep Revision'))
