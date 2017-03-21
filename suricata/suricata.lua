-- include additional path for libraries

package.path = package.path .. ";/usr/share/lua/5.1/?.lua"

--Ubuntu cpath.
package.cpath = package.cpath .. ";/usr/lib/x86_64-linux-gnu/lua/5.1/?.so"

--CentOS cpath.
package.cpath = package.cpath .. ";/usr/lib64/lua/5.1/?.so"


sha_256 = (loadfile("/opt/telepath/suricata/sha256.lua"))()
msgpack = (loadfile("/opt/telepath/suricata/msgpack.lua"))()
redisObj = (loadfile("/opt/telepath/suricata/redis.lua"))()
redis, err = redisObj:connect()

-- Using cURL as elasticsearch connector --
urlObj = require("socket.url")
local cURL = require("cURL")
-------------------------------------------

function init (args)
	local needs = {}
	needs["protocol"] = "http"
	return needs
end

block_extensions = {}
whitelist_ips = {}
--whitelist_ips[2219196937] = 2219196939
--whitelist_ips[2219196940] = 2219196942
load_balancer_ips = {}
load_balancer_headers = {}
load_cookies = {}
records = {}
record_hosts = {}
count = 0


function setup (args)
	-- Emptying the global configuration arrays --
	block_extensions = {}
	whitelist_ips = {}
	load_balancer_ips = {}
	load_balancer_headers = {}

	file = io.open("/opt/telepath/db/elasticsearch/config/connect.conf", "r")
	if (file) then
		io.input(file)
		es_location = (io.read())
		io.close(file)
	else
		es_location = "http://localhost:9200"
	end

	-- Loading filter extensions --
	login_url = es_location .. "/telepath-config/filter_extensions/extensions_id/_source"

	c = cURL.easy{
		url            = login_url,
		ssl_verifypeer = false,
		ssl_verifyhost = false,
		writefunction  = function(str)
			local tmp = string.find(str, "%[")
			local tmp2 = string.find(str, "%]")
			if (tmp and tmp2) then
				output = string.sub(str, tmp+1, tmp2-1)
				for word in string.gmatch(output, '([^",]+)') do
					block_extensions[word] = word
				end
			end
		end
	}
	c:perform()

	-- Loading whitelist ips --
	login_url = es_location .. "/telepath-config/ips/whitelist_id/_source"
	c = cURL.easy{
		url            = login_url,
		ssl_verifypeer = false,
		ssl_verifyhost = false,
		writefunction  = function(str)
			local tmp = string.find(str, "%[")
			local tmp2 = string.find(str, "%]")
			if (tmp and tmp2) then
				output = string.sub(str, tmp+1, tmp2-1)
				counter=0
				for word in string.gmatch(output, '([^:"]+)') do
					local o1,o2,o3,o4 = word:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
					if ( o1 and o2 and o3 and o4 ) then
						local int_ip = 16777216*o1 + 65536*o2 + 256*o3 + o4
						counter = counter + 1
						if (counter % 2 == 0) then
                                                        if (tmp >= int_ip) then
                                                                whitelist_ips[int_ip] = tmp
                                                        else
                                                                whitelist_ips[tmp] = int_ip
                                                        end
						else
							tmp = int_ip
						end
					end
				end
			end
		end
	}
	c:perform()

	-- Loading loadbalancer ips --
	login_url = es_location .. "/telepath-config/ips/loadbalancerips_id/_source"
	c = cURL.easy{
		url            = login_url,
		ssl_verifypeer = false,
		ssl_verifyhost = false,
		writefunction  = function(str)
			local tmp = string.find(str, "%[")
			local tmp2 = string.find(str, "%]")
			if (tmp and tmp2) then
				output = string.sub(str, tmp+1, tmp2-1)
				counter=0
				for word in string.gmatch(output, '([^:"]+)') do
					local o1,o2,o3,o4 = word:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
					if ( o1 and o2 and o3 and o4 ) then
						local int_ip = 16777216*o1 + 65536*o2 + 256*o3 + o4
						counter = counter + 1
						if (counter % 2 == 0) then
							if (tmp >= int_ip) then
								load_balancer_ips[int_ip] = tmp
							else
								load_balancer_ips[tmp] = int_ip
							end
						else
							tmp = int_ip
						end
					end


				end
			end
		end
	}
	c:perform()

	-- Loading loadbalancer headers --
	login_url = es_location .. "/telepath-config/headers/loadbalancerheaders_id/_source"
	c = cURL.easy{
		url            = login_url,
		ssl_verifypeer = false,
		ssl_verifyhost = false,
		writefunction  = function(str)
			local tmp = string.find(str, "%[")
			local tmp2 = string.find(str, "%]")
			if (tmp and tmp2) then
				output = string.sub(str, tmp+1, tmp2-1)
				for word in string.gmatch(output, '([^",]+)') do
					load_balancer_headers[word] = word
				end
			end
		end
	}
	c:perform()
	----------------------------------

	-- Loading domain cookies --
		login_url = es_location .. "/telepath-domains/domains/_search"
		c = cURL.easy{
	        url = login_url,
	        postfields = "{\"size\":10000,\"fields\":[\"AppCookieName\"],\"query\":{\"bool\":{\"must\":{\"exists\":{\"field\":\"AppCookieName\"}}}}}",
	        ssl_verifypeer = false,
	        ssl_verifyhost = false,
	        writefunction = function(str)
			while true do
				local tmp = string.find(str, "_id\"")
				if (tmp) then
					str = string.sub(str, tmp+4, string.len(str))
					tmp = string.find(str, "\"")
					local tmp2 = string.find(str, "\",")
					if(tmp2) then
						output = string.sub(str, tmp+1, tmp2-1)
					else
						break
					end
					local host = output
					tmp = string.find(str, "AppCookieName\"")
					if(tmp) then
						str = string.sub(str, tmp+14, string.len(str))
						tmp = string.find(str, "\"")
						if(tmp) then
							str = string.sub(str, tmp+1, string.len(str))
						else
							break
						end
						tmp = string.find(str, "\"")
						if(tmp) then
							output = string.sub(str, 0, tmp-1)
							load_cookies[host] = output
						else
							break
						end
					else
						break
					end
					--print(host .. "->" .. output)
					else
						break
					end
				end
			end
		}
		c:perform()

	----------------------------------

	login_url = es_location .. "/telepath-config/config/config_was_changed_id"
	c = cURL.easy{
		url            = login_url,
		ssl_verifypeer = false,
		ssl_verifyhost = false,
		postfields = "{\"value\":\"0\"}",
	}
	c:perform()

	--Debug Printings
	--for kk, vv in pairs(whitelist_ips) do
	--	print (kk .. "=====" .. vv)
	--end

	--for kk, vv in pairs(block_extensions) do
	--	print (kk .. "=====" .. vv)
	--end

	--for kk, vv in pairs(load_balancer_headers) do
	--	print (kk .. "=====" .. vv)
	--end

	--for kk, vv in pairs(load_balancer_ips) do
	--	print (kk .. "=====" .. vv)
	--end

end

local hex_to_char = function(x)
	return string.char(tonumber(x, 16))
end

local unescape = function(url)
	if (url) then
		url = url:gsub("%+"," ")
		return url:gsub("%%(%x%x)", hex_to_char)
	end
end


function log(args)

	count = count + 1
		-- Checking if the configuration was changed.
		if (count > 1000) then
		count = 0
		config_was_changed_id = redis:lpop("C")
			if (config_was_changed_id) then
				setup()
			end
	end

	local uri = HttpGetRequestUriRaw()
	uri = unescape(uri) --url decoding for uri & GET parameters.
	if not uri then
		return
	end

	query = ""
	local question_mark = string.find(uri, "?")

	if question_mark then
		uri2 = urlObj.parse(uri)
		query = uri2.query         --GET parameters.
		uri = string.sub(uri,1,question_mark-1)
	end

--	local ext = uri:reverse():find("%.")
--	if ext then
--		ext = #uri - ext+1
--		ext = string.sub(uri,ext)

--		if block_extensions[ext] == ext then
			--Extension Filter.
			--Ignoring this request.
--			return
--		end
--	end

	--Getting source IP and destination IP.
	ipver, srcip, dstip, proto, sp, dp = SCFlowTuple()

	--Making decimal client IP.
	local o1,o2,o3,o4 = srcip:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)")

	if ipver == 6 then
		--Probably IPv6
		return
	end

	int_ip = 16777216*o1 + 65536*o2 + 256*o3 + o4

--	for key, value in pairs(whitelist_ips) do
--		if (whitelist_ips[key] >= int_ip and int_ip >= key) then
			--Whitelist IP Filter.
			--Ignoring this request.
--			return
--		end
--	end

	local requestline = HttpGetRequestLine()
	if requestline then
		local tmp = string.find(requestline, " ")
		if not tmp then
			return
		end
		method = string.sub(requestline, 1, tmp-1)  --method.
	else
		--Invalid Method.
		--Ignoring this request.
		return
	end

	local responseline = HttpGetResponseLine()
	if responseline then
		local tmp = string.find(responseline, " ")
		if not tmp then
			return
		end
		status = string.sub(responseline,tmp+1,tmp+3) --status code.
	else
		--Invalid Status Code.
		--Ignoring this request.
		return
	end

	-- Checking if the configuration was changed. 
	-- config_was_changed_id = redis:lpop("C")
	-- if (config_was_changed_id) then
	-- 	setup()
	-- end

	rq_body = ""
	rs_body = ""

	a, o, e = HttpGetRequestBody(); --http request body.
	if a then
		for n, v in ipairs(a) do
			rq_body = rq_body .. v
		end
	end

	rq_body = unescape(rq_body) --url decoding for response body(POST parameters).

	a, o, e = HttpGetResponseBody(); --http response body.
	if a then
	    for n, v in ipairs(a) do
			rs_body = rs_body .. v
		end
	end

	--Time stamp
	time = SCPacketTimeString()
	month,day,year,hour,min,sec,msec = time:match("(%d+)/(%d+)/(%d+)-(%d+):(%d+):(%d+).(%d+)")
	epoch=os.time{year=year,month=month,day=day,hour=hour,min=min,sec=sec}
	epoch_str = string.format("%i",epoch)
	msec_str = string.format("%i",msec)
	if(string.len(msec_str) == 5 ) then
		epoch_str = epoch_str .. ".0" .. msec_str
	elseif(string.len(msec_str) == 4 ) then
		epoch_str = epoch_str .. ".00" .. msec_str
	elseif(string.len(msec_str) == 3 ) then
		epoch_str = epoch_str .. ".000" .. msec_str
	else
		epoch_str = epoch_str .. "." .. msec_str
	end

	request = {
		TT = epoch_str,         	--Time stamp.
		TI = srcip,			--Client IP.
		TU = uri,			--Uri.
		TB = rq_body,			--Request body.
		TP = "HTTP",			--Protocol.
		TM = method,			--Method.
		TQ = query,			--GET parameters.
		RI = dstip,			--Response IP.
		RB = rs_body,			--Response body.
		RC = status			--Status code.
	}

	rqh = HttpGetRequestHeaders()
	for k, v in pairs(rqh) do
		k = unescape(k) --url decoding for header names.
		v = unescape(v) --url decoding for header values.
		k = string.lower(k) --lowercasing for header names.

		if (k == "user-agent") then
			fp_ua = v
		elseif (k == "host") then
			fp_host = v
		end

--		if load_balancer_headers[k] == k then
--			for key, value in pairs(load_balancer_ips) do
--				if (load_balancer_ips[key] >= int_ip and int_ip >= key) then
					--Replacing to original client IP.
--					request["TI"] = v
--					srcip = v
--					o1,o2,o3,o4 = v:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
--					int_ip = 16777216*o1 + 65536*o2 + 256*o3 + o4
--				end
--			end
--		end

		if type(v) == "table" then
			request["TH_" .. k] = table.concat(v, "|&|")
		else
			request["TH_" .. k] = tostring(v)
		end
	end

	rsh = HttpGetResponseHeaders()
	for k, v in pairs(rsh) do
		k = unescape(k) --url decoding for header names.
		v = unescape(v) --url decoding for header values.
		k = string.lower(k) --lowercasing for header names.

		if (k == "set-cookie") then
			fp_cookie = v
		end

		if type(v) == "table" then
			request["RH_" .. k] = table.concat(v, "|&|")
		else
			request["RH_" .. k] = tostring(v)
		end
	end

--	if (load_cookies[fp_host]) then
--		if (fp_cookie) then
--			local cookie_tmp = string.find(fp_cookie, load_cookies[fp_host])
--			if (cookie_tmp) then
--				local fp_cookie = string.sub(fp_cookie, cookie_tmp, string.len(fp_cookie))
--				cookie_tmp = string.find(fp_cookie, "=")
--				local cookie_tmp2 = string.find(fp_cookie, ";")
--				fp_cookie = string.sub(fp_cookie, cookie_tmp+1, cookie_tmp2-1)
--				request["TS"] = sha_256.sha256(fp_cookie)
			--The statistical cookie wasn't found in the following request.
--			else
--				fingerprint = ""
--				if fp_ua then
--					fingerprint = fingerprint .. fp_ua
--				else
--					fingerprint = dstip
--				end
--
--				if fp_host then
--					fingerprint = fingerprint .. fp_host
--				else
--					fingerprint = dstip
--				end
--				request["TS"] = sha_256.sha256(srcip .. fingerprint)
--			end
		--The following request has no cookie header.
--		else
--			fingerprint = ""
--			if fp_ua then
--			fingerprint = fingerprint .. fp_ua
--			else
--				fingerprint = dstip
--			end
--
--			if fp_host then
--				fingerprint = fingerprint .. fp_host
--			else
--				fingerprint = dstip
--			end
--			request["TS"] = sha_256.sha256(srcip .. fingerprint)
--		end
--	--The load_cookies dictionary has no statistical cookie.
--	else
--		fingerprint = ""
--		if fp_ua then
--			fingerprint = fingerprint .. fp_ua
--		else
--			fingerprint = dstip
--		end
--
--		if fp_host then
--			fingerprint = fingerprint .. fp_host
--		else
--			fingerprint = dstip
--		end
--		request["TS"] = sha_256.sha256(srcip .. fingerprint)
--	end
	request["TS"] = ""
	express_flag = false

	hr_queue = redis:lpop("E")
	if hr_queue then
		hybrid_record = msgpack.unpack(hr_queue)
		if (hybrid_record.value == "0") then
			for key, val in pairs(records) do
				if (tostring(val) == hybrid_record.id) then
					records[key] = nil
					record_hosts[key] = nil
				end
			end
		else
			if (hybrid_record.mode == "i") then
				records[hybrid_record.value] = hybrid_record.id
				record_hosts[hybrid_record.value] = hybrid_record.host
			elseif (hybrid_record.mode == "u") then
				local record_url_tmp = "hybridrecord=" .. hybrid_record.value
				records[record_url_tmp] = hybrid_record.id
				record_hosts[record_url_tmp] = hybrid_record.host
			elseif (hybrid_record.mode == "s") then
				records[hybrid_record.value] = hybrid_record.id
				record_hosts[hybrid_record.value] = hybrid_record.host
			end
		end
	end

	if ( records[srcip] ) then
		local tmp = string.find(fp_host,record_hosts[srcip])
		if (tmp) then
			express_flag = true
			express_queue_id = records[srcip]
		end
	elseif ( records[ request["TS"] ] ) then
		local tmp = string.find(fp_host,record_hosts[request["TS"]])
		if (tmp) then
			express_flag = true
			express_queue_id = records[ request["TS"] ]
		end
	elseif ( records[query] ) then
		local tmp = string.find(fp_host,record_hosts[query])
		if (tmp) then
			express_flag = true
			express_queue_id = records[query]
			records[ request["TS"] ] = express_queue_id
			record_hosts[ request["TS"] ] = record_hosts[query]
		end
	end

	-- Pushing the request to redis.
	if (express_flag) then
		redis:lpush(express_queue_id,  msgpack.pack(request)  )
	else
		redis:lpush("Q",  msgpack.pack(request)  )
	end

end

function deinit (args)
	-- Shutdown
end

