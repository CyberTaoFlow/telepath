import os

file_add = '{}/output/mil_syslog.txt'.format(os.getcwd())

def save_log():
	log_dir = '{}/output/'.format(os.getcwd())
	print(log_dir)
	# IF OUTPUT FOLDER DOES NOT EXISTS, CREATE ONE!
	# os.system('mkdir {}'.format(log_dir))
	os.system("tail -n 1000 /var/log/syslog > {}mil_syslog.txt".format(log_dir))

def remove_file():
	os.system('rm {}'.format(file_add))
	print('File removed!')

if __name__ == '__main__':
	# remove_file()
	# save_log()