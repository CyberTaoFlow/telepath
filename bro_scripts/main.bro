# /usr/local/bro/share/bro/base/protocols/http

##! Implements base functionality for HTTP analysis.  The logging model is
##! to log request/response pairs and all relevant metadata together in
##! a single record.

@load base/utils/numbers
@load base/utils/files
@load base/frameworks/tunnels


module HTTP;

export {
	redef enum Log::ID += { LOG };

	## Indicate a type of attack or compromise in the record to be logged.
	type Tags: enum {
		## Placeholder.
		EMPTY
	};

	## This setting changes if passwords used in Basic-Auth are captured or
	## not.
	const default_capture_password = F &redef;

	type Info: record {
		## Timestamp for when the request happened.
		ts:                      time      &log;
		## Unique ID for the connection.
		uid:                     string  ;
		## The connection's 4-tuple of endpoint addresses/ports.
		id:                      conn_id   &log;
		## Represents the pipelined depth into the connection of this
		## request/response transaction.
		trans_depth:             count   ;

		rid:                     string  &log;

		## Verb used in the HTTP request (GET, POST, HEAD, etc.).
		method:                  string    &log &optional;
		## Value of the HOST header.
		host:                    string    &optional;
		## URI used in the request.
		uri:                     string    &log &optional;
		uri_orig :               string    &optional;

		country_code: 		string	 &log &optional;
		region: 		string	 &log &optional;
		city: 			string 	&log &optional;
		latitude: 		double 	&log &optional;
		longitude: 		double	 &log &optional;

		## Value of the "referer" header.  The comment is deliberately
		## misspelled like the standard declares, but the name used here
		## is "referrer" spelled correctly.

		## Actual uncompressed content size of the data transferred from
		## the client.
		request_body_len:        count     &log &default=0;
		## Actual uncompressed content size of the data transferred from
		## the server.
		response_body_len:       count     &log &default=0;
		## Status code returned by the server.
		status_code:             count     &log &optional;
		## Status message returned by the server.
		status_msg:              string    &log &optional;
		## Last seen 1xx informational reply code returned by the server.
		info_code:               count     &log &optional;
		## Last seen 1xx informational reply message returned by the server.
		info_msg:                string    &log &optional;
		## Filename given in the Content-Disposition header sent by the
		## server.
		filename:                string    &log &optional;
		## A set of indicators of various attributes discovered and
		## related to a particular request/response pair.
		tags:                    set[Tags] ;

		## Password if basic-auth is performed for the request.
		password:                string    &log &optional;

		## Determines if the password will be captured for this request.
		capture_password:        bool      &default=default_capture_password;

		## Indicates if this request can assume 206 partial content in
		## response.
		range_request:           bool      &default=F;

                ## Username if basic-auth is performed for the request.
                username:                string    &log &optional;
		cookie: 		set[string] 	 &optional;
		fp: 		set[string] 	 &optional;

		header_params: set[string] &log  &optional;
		get_params: set[string] &log  &optional;
		body_data:                string    &log &optional;






	};

	## Structure to maintain state for an HTTP connection with multiple
	## requests and responses.
	type State: record {
		## Pending requests.
		pending:          table[count] of Info;
		## Current request in the pending queue.
		current_request:  count                &default=0;
		## Current response in the pending queue.
		current_response: count                &default=0;
	};

	## A list of HTTP headers typically used to indicate proxied requests.
	const proxy_headers: set[string] = {
		"FORWARDED",
		"X-FORWARDED-FOR",
		"X-FORWARDED-FROM",
		"CLIENT-IP",
		"VIA",
		"XROXY-CONNECTION",
		"PROXY-CONNECTION",
	} &redef;

	## A list of HTTP methods. Other methods will generate a weird. Note
	## that the HTTP analyzer will only accept methods consisting solely
	## of letters ``[A-Za-z]``.
	const http_methods: set[string] = {
		"GET", "POST", "HEAD", "OPTIONS",
		"PUT", "DELETE", "TRACE", "CONNECT",
		# HTTP methods for distributed authoring:
		"PROPFIND", "PROPPATCH", "MKCOL",
		"COPY", "MOVE", "LOCK", "UNLOCK",
		"POLL", "REPORT", "SUBSCRIBE", "BMOVE",
		"SEARCH"
	} &redef;

	## Event that can be handled to access the HTTP record as it is sent on
	## to the logging framework.
	global log_http: event(rec: Info);
}

# Add the http state tracking fields to the connection record.
redef record connection += {
	http:        Info  &optional;
	http_state:  State &optional;
};

const cookie_keys = set("__utma","__utmb","__utmc","__utmz","__utmv");

const ports = {
	80/tcp, 81/tcp, 631/tcp, 1080/tcp, 3128/tcp,
	8000/tcp, 8080/tcp, 8888/tcp,
};
redef likely_server_ports += { ports };

# Initialize the HTTP logging stream and ports.
event bro_init() &priority=5
	{
	Log::create_stream(HTTP::LOG, [$columns=Info, $ev=log_http]);
	Analyzer::register_for_ports(Analyzer::ANALYZER_HTTP, ports);
	}

function code_in_range(c: count, min: count, max: count) : bool
	{
	return c >= min && c <= max;
	}

function new_http_session(c: connection): Info
	{
	local tmp: Info;
	tmp$ts=network_time();
	tmp$uid=c$uid;
	tmp$id=c$id;
	# $current_request is set prior to the Info record creation so we
	# can use the value directly here.
	tmp$trans_depth = c$http_state$current_request;
	return tmp;
	}

function set_state(c: connection, request: bool, is_orig: bool)
	{
	if ( ! c?$http_state )
		{
		local s: State;
		c$http_state = s;
		}

	# These deal with new requests and responses.
	if ( request || c$http_state$current_request !in c$http_state$pending )
		c$http_state$pending[c$http_state$current_request] = new_http_session(c);
	if ( ! is_orig && c$http_state$current_response !in c$http_state$pending )
		c$http_state$pending[c$http_state$current_response] = new_http_session(c);

	if ( is_orig )
		c$http = c$http_state$pending[c$http_state$current_request];
	else
		c$http = c$http_state$pending[c$http_state$current_response];
	}

event http_all_headers(c: connection , is_orig: bool , hlist: mime_header_list )
	{
		#event my_http_all_headers(is_orig,hlist,c$http$ts,addr_to_uri(c$id$orig_h),addr_to_uri(c$id$resp_h),c$http$status_code,c$http$host,c$http$method,c$http$uri,c$http$uri_orig);
	}

function no_params(uri: string): string
{

	local query="";
	if ( strstr(uri, "?") == 0)
		return query;

	query = split1(uri, /\?/)[1];

	return query;

}

function extract_params(uri: string): string
{

	local query="";
	if ( strstr(uri, "?") == 0)
		return query;

	query = split1(uri, /\?/)[2];

	return query;

}

function extract_params2(uri: string): set[string]
{
	local p: set[string] = set();

	if ( strstr(uri, "?") == 0)
		return p;

	local query: string = split1(uri, /\?/)[2];
	local opv: table[count] of string = split(query, /&/);

	# This function could easily be altered to keep order of parameter occurance by
	# altering the return type of the function and returning opv here
	#return opv;
	

	for (each in opv)
	{
		add p[ opv[each] ];
	}
	return p;
}

event deliver_request(  ts:time , ip_orig:string ,ip_resp:string ,method:string ,uri:string ,get_param: string ,header_params: set[string] ,rid:string ,c_code:string , city:string ,region:string ,latitude:double ,longitude:double ,cookie:set[string],fp:set[string]  ) &priority=5
	{

		local h_params="";
		for ( k in header_params ){
			h_params = string_cat( h_params, fmt("%s\t",k) );
		}

		local h_cookie="";
		for ( j in cookie ){
			h_cookie = string_cat( h_cookie, fmt("%s",j) );
		}

		local h_fp="";
		for ( i in fp ){
			h_fp = string_cat( h_fp, fmt("%s",i) );
		}

		event my_http_request(ts,ip_orig,ip_resp,method,uri,get_param,h_params,rid,c_code,city,region,latitude,longitude,h_cookie,h_fp);
	}

event http_request(c: connection, method: string, original_URI: string,
                   unescaped_URI: string, version: string) &priority=5
	{

	if ( ! c?$http_state )
		{
		local s: State;
		c$http_state = s;
		}

	++c$http_state$current_request;
	set_state(c, T, T);

	c$http$method = method;
	c$http$uri_orig = original_URI;

	local get_params_lower = to_lower(original_URI);

	if (!c$http?$header_params)
		c$http$header_params = set();

	c$http$uri = no_params(get_params_lower);
	local get_param: string = extract_params(get_params_lower);

	if (!c$http?$get_params)
		c$http$get_params = set();
	c$http$get_params = extract_params2(get_params_lower);

	if (!c$http?$cookie)
		c$http$cookie = set();

	if (!c$http?$fp)
		c$http$fp = set();

	local client = c$id$orig_h;
	local loc = lookup_location(client);

	if (loc?$country_code){
  		c$http$country_code = loc$country_code;
	}else{
		c$http$country_code = "00";
	}

	if (loc?$city){
  		c$http$city = loc$city;
	}else{
  		c$http$city = "Unknown";
	}

	if (loc?$region){
  		c$http$region = loc$region;
	}else{
  		c$http$region = "";
	}

	if (loc?$latitude){
  		c$http$latitude = loc$latitude;
	}else{
  		c$http$latitude = 0;
	}

	if (loc?$longitude){
  		c$http$longitude = loc$longitude;
	}else{
  		c$http$longitude = 0;
	}

	if (!c$http?$rid)
		c$http$rid = fmt("%s%d",c$http$uid,c$http$trans_depth);


	event deliver_request(c$http$ts,addr_to_uri(c$id$orig_h),addr_to_uri(c$id$resp_h),method,c$http$uri,get_param,c$http$header_params,c$http$rid,c$http$country_code,c$http$city,c$http$region,c$http$latitude,c$http$longitude,c$http$cookie,c$http$fp);

	if ( method !in http_methods )
		event conn_weird("unknown_HTTP_method", c, method);
	}


event http_entity_data(c: connection, is_orig: bool, length: count, data: string)
{
	if (is_orig)
	{
		if (!c$http?$body_data)
		{
			c$http$body_data = data;
		}
		else
		{
			c$http$body_data = string_cat( c$http$body_data, data );
		}

		if (!c$http?$rid)
			c$http$rid = fmt("%s%d",c$http$uid,c$http$trans_depth);

		event my_http_entity_data( c$http$body_data,c$http$rid) ;
	}
}

event http_reply(c: connection, version: string, code: count, reason: string) &priority=5
	{
	if ( ! c?$http_state )
		{
		local s: State;
		c$http_state = s;
		}

	# If the last response was an informational 1xx, we're still expecting
	# the real response to the request, so don't create a new Info record yet.
	if ( c$http_state$current_response !in c$http_state$pending ||
	     (c$http_state$pending[c$http_state$current_response]?$status_code &&
	       ! code_in_range(c$http_state$pending[c$http_state$current_response]$status_code, 100, 199)) )
		++c$http_state$current_response;
	set_state(c, F, F);

	c$http$status_code = code;
	c$http$status_msg = reason;
	if ( code_in_range(code, 100, 199) )
		{
		c$http$info_code = code;
		c$http$info_msg = reason;
		}

	if ( c$http?$method && c$http$method == "CONNECT" && code == 200 )
		{
		# Copy this conn_id and set the orig_p to zero because in the case of CONNECT
		# proxies there will be potentially many source ports since a new proxy connection
		# is established for each proxied connection.  We treat this as a singular
		# "tunnel".
		local tid = copy(c$id);
		tid$orig_p = 0/tcp;
		Tunnel::register([$cid=tid, $tunnel_type=Tunnel::HTTP]);
		}

	event my_http_reply(c$http$ts,c$http$uid,c$http$trans_depth);

	}

function sessionize(cookie: string, keys: set[string]) : string
    {
    local id = "";
    local fields = split(cookie, /; /);

    local matches: table[string] of string;
    for ( i in fields )
        {
        local s = split1(fields[i], /=/);
        if (s[1] !in keys)
            matches[s[1]] = s[2];
        }

        for ( key in matches )
        {
            if (id != "")
                id += ";";
            id += key + "=" + matches[key];
        }

    return id;
    }

event http_header(c: connection, is_orig: bool, name: string, value: string) &priority=5
	{
	set_state(c, F, is_orig);

	if ( is_orig ) # client headers
		{
		
		if ( name == "HOST" ){
			# The split is done to remove the occasional port value that shows up here.
			c$http$host = split1(value, /:/)[1];
			add c$http$header_params[ to_lower( fmt("%s=%s",name,value) )];
		}

		else if ( name == "COOKIE" ){
			# The split is done to remove the occasional port value that shows up here.
			add c$http$header_params[ to_lower( fmt("%s=%s",name,value) )];

			add c$http$cookie[ to_lower(sessionize(value, cookie_keys)) ];
		}

		else if ( name == "USER-AGENT" || name == "ACCEPT" || name=="ACCEPT-LANGUAGE" ){
			# The split is done to remove the occasional port value that shows up here.
			add c$http$header_params[ to_lower( fmt("%s=%s",name,value) )];

			add c$http$fp[ to_lower(value) ];
		}

		else if ( name == "AUTHORIZATION" )
			{
			if ( /^[bB][aA][sS][iI][cC] / in value )
				{
				local userpass = decode_base64(sub(value, /[bB][aA][sS][iI][cC][[:blank:]]/, ""));
				local up = split(userpass, /:/);
				if ( |up| >= 2 )
					{
					c$http$username = up[1];
					add c$http$header_params[ to_lower( fmt("%s=%s",name,up[1]) )];
					if ( c$http$capture_password )
						c$http$password = up[2];
					}
				else
					{
					c$http$username = fmt("<problem-decoding> (%s)", value);
					if ( c$http$capture_password )
						c$http$password = userpass;
					}
				}
			}
		else 
			{
			add c$http$header_params[ to_lower( fmt("%s=%s",name,value) )];
			}

		}

	}

event http_message_done(c: connection, is_orig: bool, stat: http_message_stat) &priority = 5
	{
	set_state(c, F, is_orig);

	if ( is_orig )
		c$http$request_body_len = stat$body_length;
	else
		c$http$response_body_len = stat$body_length;
	}

event http_message_done(c: connection, is_orig: bool, stat: http_message_stat) &priority = -5
	{
	# The reply body is done so we're ready to log.
	if ( ! is_orig )
		{
		# If the response was an informational 1xx, we're still expecting
		# the real response later, so we'll continue using the same record.
		if ( ! (c$http?$status_code && code_in_range(c$http$status_code, 100, 199)) )
			{
			Log::write(HTTP::LOG, c$http);
			delete c$http_state$pending[c$http_state$current_response];
			}
		}
	}

event connection_state_remove(c: connection) &priority=-5
	{
	# Flush all pending but incomplete request/response pairs.
	if ( c?$http_state )
		{
		for ( r in c$http_state$pending )
			{
			# We don't use pending elements at index 0.
			if ( r == 0 ) next;
			Log::write(HTTP::LOG, c$http_state$pending[r]);
			}
		}
	}



