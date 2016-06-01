ngx.ctx.redis = redis:new()

-- Generates random strings for us
function RandomString(l)
        if l < 1 then return nil end -- Check for l < 1
        local s = "" -- Start string
        for i = 1, l do
            n = math.random(97, 122) -- Generate random number from 32 to 126
            s = s .. string.char(n) -- turn it into character and add to string
        end
        return s -- Return string
end

-- Global redis connection

local ok, err = ngx.ctx.redis:connect("127.0.0.1", 6379)
if not ok then
  ngx.log(ngx.ERR, "Cant connect to Redis")
end

local ok, err = ngx.ctx.redis:set_keepalive(10000, 100)
if not ok then
  ngx.log(ngx.ERR, "Failed to set keepalive")
end

ngx.ctx.request = {}

-- Connect to check captcha state
local redis = require "resty.redis"
local red = redis:new()
red:set_timeout(1000) -- 1 sec
local ok, err = red:connect("127.0.0.1", 6379)
if not ok then
   return
end
local res, err = red:get("CAPTCHA_" .. ngx.var.remote_addr)



if res ~= nil then

	-- No captcha was set for this user, return to normal flow
	if res == ngx.null then
		do return end
	else
		-- Have a captcha to validate against
		if res ~= "" then
			local args, err = ngx.req.get_post_args()
			if args and args["captcha"] and res == args["captcha"] then
			
				red:del("CAPTCHA_" .. ngx.var.remote_addr)
				ngx.header["Content-Type"] = "text/html"
				ngx.header["cache_control"] = "no-cache"
				ngx.status = 200
				ngx.print('<html><head><meta http-equiv="refresh" content="1"></head><body>Redirecting..</body></html>')
				return ngx.exit(ngx.HTTP_OK)
				
			end
		end
		
		-- Generate new captcha
		local captcha = RandomString(5)

		-- Update redis with the correct captcha for this IP
		red:set("CAPTCHA_" .. ngx.var.remote_addr, captcha)

		-- Run captcha generating script with our new key
		local f = assert(io.popen("php /opt/telepath/ui/captcha.php " .. captcha, 'r'))
		local s = assert(f:read('*a'))
		f:close()

		-- Return captcha from php
		ngx.header["Content-Type"] = "text/html"
		ngx.header["cache_control"] = "no-cache"
		ngx.status = 200
		ngx.print(s)

		return ngx.exit(ngx.HTTP_OK)
		
	end


end
