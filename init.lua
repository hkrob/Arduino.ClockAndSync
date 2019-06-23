print("LUA Interpreter started")
    gpio.mode(6,gpio.OUTPUT)
    gpio.write(6,gpio.LOW)

wifi.setmode(wifi.STATION)
print ("ESP8266 mode is: " ..  wifi.getmode())
wifi.sta.config ( "APNAME" , "PASSWORD" )
print("Attempting to obtain IP")
tmr.alarm(1,5000, 1, function() 
    if wifi.sta.getip()==nil then 
        print("NOK Waiting for IP Address ...") 
    else 
        print("OK Connected - IP address : "..wifi.sta.getip()) 
        tmr.stop(1)
		gpio.write(6,gpio.HIGH)		
    -- further code could go here
    end 
end)


-- NodeMCU pin mappings https://github.com/nodemcu/nodemcu-firmware