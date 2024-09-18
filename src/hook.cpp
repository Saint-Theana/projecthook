#include <map>
#include <string>
#include "format.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include "common/milog.h"
#include "common/minet.h"
#include <dlfcn.h>
#include "google/protobuf/util/json_util.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <libgen.h>   // For dirname()
#include <unistd.h>   // For readlink()
#include <list>
#include "httplib.h"
#include "pb2json.h"
#include "app-types.h"
#include "PFishHook.h"
#include "Zydis/Zydis.h"
#define __int64 long long


#define STORE_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) RETURN_T (*NAME) PARAMS
#define DO_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) RETURN_T (*NAME) PARAMS
#include "functions.h"
#undef DO_APP_FUNC
#undef STORE_APP_FUNC

std::string region_name="";

std::list<std::string>& getCmdNameFilterList() {
    static std::list<std::string> cmd_name_filter_list;
    return cmd_name_filter_list;
}

std::string& get_api_server() {
    static std::string api_server = "initial_value";
    return api_server;
}

std::string& get_api_path() {
    static std::string api_path = "initial_value";
    return api_path;
}

__int64 convertPacketToString_Fake(std::shared_ptr<common::minet::Packet> packet_ptr, std::string *name){
    
    if(region_name.empty()){
        GameserverService *service =findService();
        Config *config=getConfig(service).get();
        std::string *regionName =getRegionName(config);
        region_name=*regionName;
    }
    
    common::minet::Packet *packet = packet_ptr.get();
    std::string cmd_name = *getCmdName(packet->cmd_id);
    
    
    std::list<std::string>& cmd_name_filter_list = getCmdNameFilterList();
  
    auto it = std::find(cmd_name_filter_list.begin(), cmd_name_filter_list.end(), cmd_name);

    if (it != cmd_name_filter_list.end()) {
        proto::PacketHead& head=packet->head_;
        std::shared_ptr<google::protobuf::Message> message_ptr;
        getProto(&message_ptr,packet);
        google::protobuf::Message *message=message_ptr.get();
        nlohmann::json json;
        unsigned int uid =head.user_id();
        Pb2Json::Message2Json(*message,json,true);
        std::string bodystr=json.dump();
        
        INFO("uid: %d,cmd: %s. found in cmd_name_filter_list,reporting\n" ,uid, cmd_name.c_str());
        httplib::Client cli(get_api_server());
        httplib::Result res = cli.Post(str_format("%s?region=%s&uid=%d&cmd_name=%s",get_api_path().c_str(),region_name.c_str(),uid,cmd_name.c_str()),bodystr, "text/plain");
        
    } 

    return convertPacketToString(packet_ptr,name);
}

std::string getCurrentDirPath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char *path;
    if (count != -1) {
        path = dirname(result);
    }
    return std::string(path);
}

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        fprintf(stderr, "Failed to open file: %s\n", filePath.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

__attribute__((constructor)) void setup_hook() {
    // 获取原始 foo 函数地址
    INFO("Start hooking...\n");
    std::string currentDir = getCurrentDirPath();
    INFO("%s \n" ,currentDir.c_str() );
    if (currentDir.empty()) {
        fprintf(stderr, "Failed to get current directory path.\n");
        return;
    }

    std::string filename = "config.json"; // Replace with your file name
    std::string filePath = currentDir + "/" + filename;

    std::string fileContent = readFile(filePath);
    if (fileContent.empty()) {
        fprintf(stderr,  "Failed to read file: %s. \n" , filePath.c_str() );
        return;
    }

    INFO("read config succ,content : %s. \n" , fileContent.c_str());
    nlohmann::json json;
    std::ifstream i(filePath);
    i>>json;
    
    
    
    get_api_server()=json["api_server"].get<std::string>();
    get_api_path()=json["api_path"].get<std::string>();

    int size=json["cmd_name_filter"].size();
    
    INFO("read config succ,content : %d. ",size);
    
    std::list<std::string>& cmd_name_filter_list = getCmdNameFilterList();
  
    for(int i = 0; i < size; i++)
    {
        cmd_name_filter_list.push_back(json["cmd_name_filter"][i].get<std::string>());
    }
    
    void *handle = dlopen(NULL, RTLD_NOW);
    
    #define DO_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) NAME = (RETURN_T (*) PARAMS) dlsym(handle, OFFSET);
    #define STORE_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) FakeIt(dlsym(handle, OFFSET),(void**)&NAME);
    #include "functions.h"
    #undef DO_APP_FUNC
    #undef STORE_APP_FUNC
    
    INFO("Hook done!\n");
}


// ---
extern "C" std::map<std::string,std::string> getFuncMap() {
   std::map<std::string,std::string> hookFuncMap ={
    {
      "_ZN10ProtoUtils21convertPacketToStringESt10shared_ptrIN6common5minet6PacketEERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",
      "_Z26convertPacketToString_FakeSt10shared_ptrIN6common5minet6PacketEEPNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE"
    
    }
  };
  return hookFuncMap;
}

// ---


