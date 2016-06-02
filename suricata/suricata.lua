-- include additional path for libraries

package.path = package.path .. ";/usr/share/lua/5.1/?.lua"

--Ubuntu cpath.
package.cpath = package.cpath .. ";/usr/lib/x86_64-linux-gnu/lua/5.1/?.so"

--CentOS cpath.
package.cpath = package.cpath .. ";/usr/lib64/lua/5.1/?.so"


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

setting_counter = 0
block_extensions = {}
whitelist_ips = {}
--whitelist_ips[2219196937] = 2219196939
--whitelist_ips[2219196940] = 2219196942
load_balancer_ips = {}
load_balancer_headers = {}



function setup (args) 
	-- Emptying the global configuration arrays --
	block_extensions = {}
	whitelist_ips = {}
	load_balancer_ips = {}
	load_balancer_headers = {}

	-- Loading filter extensions --
	login_url = "http://localhost:9200/telepath-config/filter_extensions/extensions_id/_source"
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
	login_url = "http://localhost:9200/telepath-config/ips/whitelist_id/_source"
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
	login_url = "http://localhost:9200/telepath-config/ips/loadbalancerips_id/_source"
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
	login_url = "http://localhost:9200/telepath-config/headers/loadbalancerheaders_id/_source"
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
	
	login_url = "http://localhost:9200/telepath-config/config/config_was_changed_id"
	c = cURL.easy{
		url            = login_url,
		ssl_verifypeer = false,
		ssl_verifyhost = false,
		postfields = "{\"value\":\"0\"}",
	}
	c:perform()	

	--Debug Printings
        --for kk, vv in pairs(whitelist_ips) do
        --        print (kk .. "=====" .. vv)
        --end

	--for kk, vv in pairs(block_extensions) do
	--	print (kk .. "=====" .. vv)
	--end

        --for kk, vv in pairs(load_balancer_headers) do
        --        print (kk .. "=====" .. vv)
        --end

        --for kk, vv in pairs(load_balancer_ips) do
        --        print (kk .. "=====" .. vv)
        --end


end

local hex_to_char = function(x)
	return string.char(tonumber(x, 16))
end

local unescape = function(url)
	if (url) then
		return url:gsub("%%(%x%x)", hex_to_char)
	end
end

function log(args)
	-- Checking if the configuration was changed. 
	if setting_counter == 100 then
		login_url = "http://localhost:9200/telepath-config/config/config_was_changed_id/_source"
		c = cURL.easy{
			url            = login_url,
			ssl_verifypeer = false,
			ssl_verifyhost = false,
			writefunction  = function(str)
				local tmp = string.find(str, "1")
				if ( tmp ) then
					setup()
				end
			end
		}
                c:perform()
		setting_counter = 0
	else
	   setting_counter = setting_counter + 1
	end

	-- Read it!
	ipver, srcip, dstip, proto, sp, dp = SCFlowTuple()
	
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


	local requestline = HttpGetRequestLine()
	if requestline then
		local tmp = string.find(requestline, " ")
		method = string.sub(requestline, 1, tmp-1)  --method.
	else
		return
	end

	local responseline = HttpGetResponseLine()
	if responseline then
		local tmp = string.find(responseline, " ")
		status = string.sub(responseline,tmp+1,tmp+3) --status code.
	else
		return
	end

	local uri = HttpGetRequestUriRaw()
	uri = unescape(uri) --url decoding for uri & GET parameters.
	
	-- uri = string.lower(uri)			
	query = ""
	tmp = string.find(uri, "?")

	if tmp then
		uri2 = urlObj.parse(uri)
		query = uri2.query	   --GET parameters.
		uri = string.sub(uri,1,tmp-1)
	end

	uri = string.lower(uri)

	local ext = uri:reverse():find("%.")
	if ext then
		ext = #uri - ext+1
		ext = string.sub(uri,ext)

		if block_extensions[ext] == ext then
			--Ignoring this request.
			return
		end
	end

	--Time stamp
	time = SCPacketTimeString()
	month,day,year,hour,min,sec = time:match("(%d+)/(%d+)/(%d+)-(%d+):(%d+):(%d+)")
	epoch=os.time{year=year,month=month,day=day,hour=hour,min=min,sec=sec}

	request = {
		TT = string.format("%i",epoch),	--Time stamp.
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

	--Making decimal client IP.
	local o1,o2,o3,o4 = srcip:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
	int_ip = 16777216*o1 + 65536*o2 + 256*o3 + o4

	rqh = HttpGetRequestHeaders()
	for k, v in pairs(rqh) do
		k = string.lower(k)

		if load_balancer_headers[k] == k then
			for key, value in pairs(load_balancer_ips) do
				if (load_balancer_ips[key] >= int_ip and int_ip >= key) then
					--Replacing to original client IP.
					request["TI"] = v 
					o1,o2,o3,o4 = v:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
					int_ip = 16777216*o1 + 65536*o2 + 256*o3 + o4
				end
			end
		end

		if type(v) == "table" then
			request["TH_" .. k] = table.concat(v, "|&|")
		else
			request["TH_" .. k] = tostring(v)
		end
	end

	for key, value in pairs(whitelist_ips) do
		if (whitelist_ips[key] >= int_ip and int_ip >= key) then
			--Ignoring this request.
			return
		end
	end

	rsh = HttpGetResponseHeaders()
	for k, v in pairs(rsh) do
		k = unescape(k) --url decoding for header names.
		v = unescape(i) --url decoding for header values.
		k = string.lower(k) --lowercasing for header names.

		if type(v) == "table" then
			request["RH_" .. k] = table.concat(v, "|&|")
		else
			request["RH_" .. k] = tostring(v)
		end
	end

	-- Pushing the request to redis.
	redis:lpush("Q",  msgpack.pack(request)  )
	
end

function deinit (args)
	-- Shutdown
end
