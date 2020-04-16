local http=require("socket.http")
local json=require('common/json')

WebClient={}

function WebClient:request(url)
    local response,code=http.request(url)
    print('------response=', response)
    print('------code=', code)
end
