-- Inject a script
tmp = ngx.arg[1]
-- replacestr = "<script>console.log(\'Proxied by Telepath\');</script></head>" 
-- tmp = ngx.re.sub(tmp,"</head>", replacestr)

-- Modify response
-- ngx.arg[1] = tmp

-- Store a copy for the engine
if not ngx.ctx.request.RB then
	ngx.ctx.request.RB = tmp
else
	ngx.ctx.request.RB = ngx.ctx.request.RB .. tmp
end
