-- yahooQuoteRetrieval.lua
local parm={...}
conn = nil
conn=net.createConnection(net.TCP, 0)
Tstart  = tmr.now()
-- show the retrieved web page
conn:on("receive", function(conn, payload) 
                       success = true
                       --print(payload)
print('###,')
local quote = (string.sub(payload,string.find(payload,parm[1]),string.find(payload,parm[1])+100))
print (quote)		 
print(',###')
                       end) 
-- when connected, request page (send parameters to a script)
conn:on("connection", function(conn, payload) 
                       conn:send("GET /d/quotes.csv?s="..parm[1].."&f=nsl1opd1t1np2&e=.csv"
                        .."T="..(tmr.now()-Tstart)
                        .."&heap="..node.heap()
                        .." HTTP/1.1\r\n" 
                        .."Host: download.finance.yahoo.com\r\n" 
               		   .."Connection: close\r\n"
                        .."Accept: */*\r\n" 
                        .."User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n" 
                        .."\r\n")
                       end) 
-- when disconnected
conn:on("disconnection", function(conn, payload) 
end)
                                             
conn:connect(80,'download.finance.yahoo.com')
-- yahoo quote formatting options: https://code.google.com/p/yahoo-finance-managed/wiki/enumQuoteProperty
-- to call: loadfile("quoteRetrieval.lua") ( "0001.HK" )