function init (args)
    local needs = {}
    needs["payload"] = tostring(true)
    return needs
end

function match(args)
    local p = args['payload']
    if p == nil then
        --print ("no payload")
        return 0
    end
 
    if #p < 8 then
        --print ("payload too small")
    end
    if (p:byte(1) ~= 24) then
        --print ("not a heartbeat")
        return 0
    end
 
    -- message length
    len = 256 * p:byte(4) + p:byte(5)
    --print (len)
 
    -- heartbeat length
    hb_len = 256 * p:byte(7) + p:byte(8)

    -- 1+2+16
    if (1+2+16) >= len  then
        print ("invalid length heartbeat")
        return 1
    end

    -- 1 + 2 + payload + 16
    if (1 + 2 + hb_len + 16) > len then
        print ("heartbleed attack detected: " .. (1 + 2 + hb_len + 16) .. " > " .. len)
        return 1
    end
    --print ("no problems")
    return 0
end
return 0
