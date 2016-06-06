function lines(str)
  local t = {}
  local function helper(line) table.insert(t, line) return "" end
  helper((str:gsub("(.-)\r?\n", helper)))
  return t
end

function makeString(l)
        if l < 1 then return nil end -- Check for l < 1
        local s = "" -- Start string
        for i = 1, l do
                s = s .. string.char(math.random(32, 64)) -- Generate random number from 32 to 126, turn it into character and add to string
        end
        return s -- Return string
end

function explode(d,p)
  local t, ll
  t={}
  ll=0
  if(#p == 1) then return {p} end
    while true do
      l=string.find(p,d,ll,true) -- find the next d in the string
      if l~=nil then -- if "not not" found then..
        table.insert(t, string.sub(p,ll,l-1)) -- Save it in our array.
        ll=l+1 -- save just after where we found it for searching next time.
      else
        table.insert(t, string.sub(p,ll)) -- Save what's left in our array.
        break -- Break at end, as it should be, according to the lua manual.
      end
    end
  return t
end

function tprint(tbl, indent)
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    elseif type(v) == 'boolean' then
      print(formatting .. tostring(v))      
    else
      print(formatting .. v)
    end
  end
end

function try(f, catch_f)
	local status, exception = pcall(f)
	if not status then
		catch_f(exception)
	else
	print(status)
	end
end

function stringTrim(s)
    assert(s ~= nil, "String can't be nil")
    return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end

function loadINI()
	path = '/opt/telepath/conf/atms.conf'
	assert(path ~= nil, "Path can't be nil")
	local f = io.open(path, "r")
	local tab = {}
	local line = ""
	local newLine
	local i
	local currentTag = nil
	local found = false
	local pos = 0
	while line ~= nil do
			found = false           
			line = line:gsub("\\;", "#_!36!_#") -- to keep \;
			line = line:gsub("\\=", "#_!71!_#") -- to keep \=
			if line ~= "" then
					-- Delete comments
					newLine = line
					line = ""
					for i=1, string.len(newLine) do                         
							if string.sub(newLine, i, i) ~= ";" then
									line = line..newLine:sub(i, i)                                          
							else                            
									break
							end
					end
					line = stringTrim(line)
					-- Find key and values
					if not found and line ~= "" then                                
							pos = line:find("=")                            
							if pos == nil then
									error("Bad INI file structure")
							end
							line = line:gsub("#_!36!_#", ";")
							line = line:gsub("#_!71!_#", "=")
							tab[stringTrim(line:sub(1, pos-1))] = stringTrim(line:sub(pos+2, line:len()-1))
							found = true                    
					end                     
			end
			line = f:read("*line")          
	end
	f:close()
	return tab
end