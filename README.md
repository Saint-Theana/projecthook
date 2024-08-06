#Project Hook
##This is a project for hooking a server program of a certain Anime Game called "GayShit Inffect"

##What is this for?
###Report certain packet to http api in json format.

##How to build?
```shell
make
```

##How to use?
###configure your config file,save as config.json,put it under same directory as where gameserver is.edit cmd_name_filter with the packet you wish to report.
```json
{
  "cmd_name_filter": [
    "DoGachaRsp"
  ],
  "api_server":"http://127.0.0.1:21000",
  "api_path":"/packet_report"
}
```
```shell
#to be idiotic,libasan.so must be first to load,and than it is our beloved little trick.
LD_PRELOAD=../lib/libasan.so.5:./libhook.so ./gameserver -i yourappid
```
 
##tested on 3.2