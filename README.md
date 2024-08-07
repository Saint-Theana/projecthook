# Project Hook</br>
## This is a project for hooking a server program of a certain Anime Game called "GayShit Inffect"

## What is this for?</br>
### Report certain packet to http api in json format.

## How to build?
```shell
make
```

## How to use?</br>
### configure your config file,save as config.json,put it under same directory as where gameserver is.edit cmd_name_filter with the packet you wish to report.
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
#mkdir -p patch/lib
#echo 1162732_test.so > patch/installed
#copy 1162732_test.so to patch/lib/1162732_test.so
├── gameserver
└── patch
    ├── installed
    └── lib
        └── 1162732_test.so
```
 
## tested on 3.2
## it reports proto message in json format with region name and uid & cmd_name.
## report example:
```text
::ffff:127.0.0.1 - - [07/Aug/2024 00:00:23] "POST /packet_report?region=dev_gio_32&uid=1000&cmd_name=DoGachaRsp HTTP/1.1" 200 -
```
```json
{
  "gachaTimesLimit": 4294967295,
  "tenCostItemNum": 10,
  "gachaTimes": 1,
  "gachaScheduleId": 913004,
  "leftGachaTimes": 4294967295,
  "tenCostItemId": 223,
  "costItemNum": 1,
  "newGachaRandom": 195,
  "gachaType": 202,
  "costItemId": 223,
  "gachaItemList": [
    {
      "gachaItem": {
        "itemId": 1020,
        "count": 1
      },
      "transferItems": [
        {
          "item": {
            "itemId": 1120,
            "count": 1
          }
        },
        {
          "item": {
            "itemId": 221,
            "count": 2
          }
        }
      ]
    }
  ],
  "curScheduleDailyGachaTimes": 32,
  "dailyGachaTimes": 56
}
```

## Thanks: </br>
### Hotaru for tips of how to use internal hot_patch and internal milog api.
### [cpp-httplib](https://github.com/yhirose/cpp-httplib)