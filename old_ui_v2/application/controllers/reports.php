<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Reports extends CI_Controller {
	
	function sendmail() {
		
		$config = get_smtp_config();
		$message = '';
		$this->load->library('email', $config);
		$this->email->set_newline("\r\n");
		$this->email->from('hibirdsec@gmail.com'); // change it to yours
		$this->email->to('ndkeren@gmail.com');// change it to yours
		$this->email->subject('Resume from JobsBuddy for your Job posting');
		$this->email->message($message);
		
		if($this->email->send()) {
			echo 'Email sent.';
		} else {
			show_error($this->email->print_debugger());
		}

	}
	
	function index() {
		
		echo 'Welcome to TP reports controller';
		
		echo 'SMTP Details are as following >>';
		
		$this->load->model('ConfigModel');
		$conf = $this->ConfigModel->get();
		
		echo 'SMTP IP   :: ' . $conf['smtp_ip'] . '<br>';
		echo 'SMTP PORT :: ' . $conf['smtp_port'] . '<br>';
		echo 'SYSLOG    :: ' . $conf['remote_syslog_ip'] . '<br>';
		
		echo 'Our Query';
			
		$this->db->select('DISTINCT(alerts.SID), alerts.date,alerts.IP,rule_groups.name,rule_groups.description,pages.display_path,applications.app_domain,alerts.average_score,rule_groups.email_recipient,email,syslog,session_alerts.RID,rule_groups.alert_param_ids,msg_id', false);
		$this->db->from('alerts,rule_groups,session_alerts,pages,applications');
		$this->db->where('alerts.rule_group=rule_groups.id', null, false);
		$this->db->where('session_alerts.rule_group=alerts.rule_group', null, false);
		$this->db->where('session_alerts.SID=alerts.SID', null, false);
		$this->db->where('session_alerts.page_id=pages.page_id', null, false);
		$this->db->where('applications.app_id=alerts.hostname', null, false);
		$this->db->where('applications.app_id=pages.app_id', null, false);
		//$this->db->where('(email=1 or syslog=1)', null, false);
		$this->db->limit(100);
		
		$query = $this->db->get();
		$result = $query->result();
		
		print_r($result);
		
		
		
		echo $this->db->last_query();
		
		/*
		

	for result in s.run(query1):
		if not result[12]:

			html = """\
			<html>
				<head></head>
				<body>
				<table border="1">
					<tr style="font-weight:bold">
						<td>Alert ID</td> <td>Time</td> <td>Source IP</td> <td>Alert Name</td> <td>Alert Description</td> <td>Page</td> <td>Domain</td> <td>Score</td> 
					</tr>

			"""
			html +=  "\t\t\t<tr>\n\t\t\t\t<td>"+str(result[13])+"</td> <td>"+ time.strftime("%a, %d %b %Y %H:%M:%S",time.localtime(float(result[1]))) + "</td> <td>"+result[2]+"</td> <td>"+result[3]+"</td> <td>"+result[4]+"</td> <td>"+result[5]+"</td> <td>"+result[6]+"</td> <td>"+str(result[7])+"</td>  \n\t\t\t</tr>\n"	
			html +="\t\t</table>\n\t\t</body>\n\t</html>"
			syslogdata =  str(result[13])+" | "+time.strftime("%a, %d %b %Y %H:%M:%S",time.localtime(float(result[1]))) + " | "+result[2]+" | "+result[3]+" | "+result[4]+" | "+result[5]+" | "+result[6]+" | "+str(result[7])

			if(result[9]==1):
				recipient = str(result[8])
				reclist = recipient.split(",")
				for rec1 in reclist:
					session = smtplib.SMTP(IP_, PORT)
					session.ehlo()
					headers = ["From: " + sender,"Subject: " + subject,"To: " + rec1,"MIME-Version: 1.0","Content-Type: text/html"]
					headers = "\r\n".join(headers)
					session.sendmail(sender, rec1 , headers + "\r\n\r\n" + html)
					session.quit()

			if( (result[10]==1) ):
				logger.debug(syslogdata)
		else:
			html = """\
			<html>
				<head></head>
				<body>
				<table border="1">
					<tr style="font-weight:bold">
						<td>Alert ID</td> <td>Time</td> <td>Source IP</td> <td>Alert Name</td> <td>Alert Description</td> <td>Page</td> <td>Domain</td> <td>Score</td> <td>Parameter</td> <td>Value</td>
					</tr>

			"""
	
			param_ids = result[12].split(",")
			logger = logging.getLogger("Telepath")	
			for param_id in param_ids:

				vals = s.run("SELECT data,(SELECT att_name FROM attributes WHERE att_id='"+str(param_id)+"') FROM attribute_scores WHERE att_id='"+str(param_id)+"' AND RID="+str(result[11])+";")
				html +=  "\t\t\t<tr>\n\t\t\t\t<td>"+str(result[13])+"</td> <td>"+ time.strftime("%a, %d %b %Y %H:%M:%S",time.localtime(float(result[1]))) + "</td> <td>"+result[2]+"</td> <td>"+result[3]+"</td> <td>"+result[4]+"</td> <td>"+result[5]+"</td> <td>"+result[6]+"</td> <td>"+str(result[7])+"</td> <td>"+vals[0][1]+"</td> <td>"+vals[0][0]+"</td> \n\t\t\t</tr>\n"	

				if(result[10]==1):
					syslogdata2 =  str(result[13])+" | "+time.strftime("%a, %d %b %Y %H:%M:%S",time.localtime(float(result[1]))) + " | "+result[2]+" | "+result[3]+" | "+result[4]+" | "+result[5]+" | "+result[6]+" | "+str(result[7])+" | "+vals[0][1]+" | "+vals[0][0]
					logger.debug(syslogdata2)

			html +="\t\t</table>\n\t\t</body>\n\t</html>"

			if(result[9]==1):
				recipient = str(result[8])
				reclist = recipient.split(",")
				for rec1 in reclist:
					session = smtplib.SMTP(IP_, PORT)
					session.ehlo()
					headers = ["From: " + sender,"Subject: " + subject,"To: " + rec1,"MIME-Version: 1.0","Content-Type: text/html"]
					headers = "\r\n".join(headers)
					session.sendmail(sender, rec1 , headers + "\r\n\r\n" + html)
					session.quit()

			query2 = "UPDATE alerts SET email=0,syslog=0 WHERE email=1 OR syslog=1"
			s.run(query2)

		*/
	}

}

?>