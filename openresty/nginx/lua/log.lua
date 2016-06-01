local function log(premature, redis, request)
				
  msgpack = (loadfile("/opt/telepath/openresty/nginx/lua/msgpack.lua"))()
  ngx.ctx.redis = redis:new()
  local ok, err = ngx.ctx.redis:connect("127.0.0.1", 6379)
  if not ok then
	ngx.log(ngx.ERR, "Cant connect to Redis")
  end
	
  ok, err = ngx.ctx.redis:lpush("Q", msgpack.pack(request))

end

-- Status code
ngx.ctx.request.RC = string.format("%i", ngx.status);
-- Time
ngx.ctx.request.TT = string.format("%8.4f", ngx.req.start_time());

-- Method
ngx.ctx.request.TM = ngx.req.get_method()
-- Client IP
ngx.ctx.request.TI = ngx.var.remote_addr
-- Response IP
ngx.ctx.request.RI = ngx.var.server_addr
-- Request Body
ngx.ctx.request.TB = ngx.var.request_body
-- Request URI
ngx.ctx.request.TU = ngx.var.uri

if ngx.var.https == "on" then
        ngx.ctx.request.TP = "https"
end
			
local request_headers  = ngx.req.get_headers()
local response_headers = ngx.resp.get_headers()
local request_query    = ngx.req.get_uri_args()

for k, v in pairs(request_headers) do
	if type(v) == "table" then
							ngx.ctx.request["TH_" .. k] = table.concat(v, "|&|")
					else
							ngx.ctx.request["TH_" .. k] = tostring(v)
					end
end

for k, v in pairs(response_headers) do
	if type(v) == "table" then
							ngx.ctx.request["RH_" .. k] = table.concat(v, "|&|")
					else
							ngx.ctx.request["RH_" .. k] = tostring(v)
					end
end

if request_query then
	for k, v in pairs(request_query) do

	if type(v) == "table" then
		ngx.ctx.request["TG_" .. k] = table.concat(v, "|&|")
			else
		ngx.ctx.request["TG_" .. k] = tostring(v)
			end

	end
end

local ok, err = ngx.timer.at(0, log, ngx.ctx.redis, ngx.ctx.request)
if not ok then
  ngx.log(ngx.ERR, "failed to create timer: ", err)
  return
end
