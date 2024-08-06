#include <map>
#include <string>
#include "format.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include "common/milog.h"
#include "common/minet.h"
#include "gothook.h"
#include <dlfcn.h>
#include "google/protobuf/util/json_util.h"
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <libgen.h>   // For dirname()
#include <unistd.h>   // For readlink()
#include <list>
#include "httplib.h"

#define __int64 long long

LIBDETOUR_DECL_TYPE(__int64,  convertPacketToString, std::shared_ptr<common::minet::Packet>, std::string *);
LIBDETOUR_DECL_TYPE(std::string *,  getProtoDebugString, std::string *,common::minet::Packet *);
LIBDETOUR_DECL_TYPE(std::string *,  DebugString, std::string *,google::protobuf::Message *);


libdetour_ctx_t detour_ctx_convertPacketToString;

libdetour_ctx_t detour_ctx_getProtoDebugString;

libdetour_ctx_t detour_ctx_DebugString;

void *convertPacketToString;

void *getProtoDebugString;

void *DebugString;

typedef std::string *(*getCmdName_ptr)(uint32_t);

getCmdName_ptr getCmdName;

struct GameserverService;
struct Config;
typedef GameserverService *(*findService_ptr)();

findService_ptr findService;

typedef std::shared_ptr<Config> (*getConfig_ptr)(GameserverService *);

getConfig_ptr getConfig;

typedef std::string *(*getRegionName_ptr)(Config *);

getRegionName_ptr getRegionName;

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


std::string *DebugString_Fake(std::string *retstr,google::protobuf::Message *message){
    //std::cout << "DebugString_Fake" << std::endl;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true; // 设置为 true 启用缩进

    google::protobuf::util::MessageToJsonString(*message,retstr,options);
    //std::cout << "DebugString_Fake 2" << std::endl;
    
    return retstr;
    /*std::string b;
    google::protobuf::util::MessageToJsonString(*message,&b);
    std::cout << b << std::endl;
    std::string *ret;
    LIBDETOUR_ORIG_GET(&detour_ctx_DebugString, ret, DebugString,retstr , message);
    return ret;*/
  /*  std::string b;
    std::cout << "DebugString_Fake 1" << std::endl;
    
    google::protobuf::util::MessageToJsonString(*message,&b);
    std::cout << "DebugString_Fake 2" << std::endl;
    
    std::cout << b << std::endl;*/
    /*std::string *ret;
    LIBDETOUR_ORIG_GET(&detour_ctx_DebugString, ret, DebugString,retstr , message);
    return ret;*/
}

std::string *getProtoDebugString_Fake(std::string *retstr,common::minet::Packet *const _this){
 
    std::string *ret;
    LIBDETOUR_ORIG_GET(&detour_ctx_getProtoDebugString, ret, getProtoDebugString,retstr , _this);
    return retstr;
}



__int64 convertPacketToString_Fake(std::shared_ptr<common::minet::Packet> packet_ptr, std::string *name){
    
    (*name).clear();

    if(region_name.empty()){
        GameserverService *service =findService();
        Config *config=getConfig(service).get();
        std::string *regionName =getRegionName(config);
        region_name=*regionName;
    }
    
    common::minet::Packet *packet = packet_ptr.get();
    
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true; // 设置为 true 启用缩进
    
    std::string headstr="";
    proto::PacketHead& head=packet->head_;
    google::protobuf::util::MessageToJsonString(head,&headstr,options);
    
    std::string bodystr="";
    getProtoDebugString_Fake(&bodystr,packet);
    
    std::string cmd_name = *getCmdName(packet->cmd_id);
    unsigned int uid =packet->head_.user_id();
    unsigned int client_sequence_id=packet->head_.client_sequence_id();
    unsigned int packet_id=packet->head_.packet_id();
    unsigned int rpc_id=packet->head_.rpc_id();
    unsigned int sent_ms=packet->head_.sent_ms();
    
    std::string packet_info= str_format(
        "uid:%u, cmd_id:%u, cmd_name:%s, packet_id:%u, rpc_id:%u, client_sequence_id:%lu sent_ms:%lu",
        uid,
        packet->cmd_id,
        cmd_name.c_str(),
        packet_id,
        rpc_id,
        client_sequence_id,
        sent_ms
    );
    
    std::string finalstr=packet_info+"\nHEAD: "+headstr+"\nBODY: "+bodystr+"\n";
    
    (*name).append(finalstr);
    std::list<std::string>& cmd_name_filter_list = getCmdNameFilterList();
  
    auto it = std::find(cmd_name_filter_list.begin(), cmd_name_filter_list.end(), cmd_name);

    if (it != cmd_name_filter_list.end()) {
        fprintf(stdout, "cmd: %s. found in cmd_name_filter_list\n" , cmd_name.c_str());
        fprintf(stdout, "api_path: %s. \n" , get_api_path().c_str());
        fprintf(stdout, "api_path: %s. \n" , get_api_server().c_str());
        
        httplib::Client cli(get_api_server());
        httplib::Result res = cli.Post(str_format("%s?region=%s&uid=%d",get_api_path().c_str(),region_name.c_str(),uid),bodystr, "text/plain");
        
    } 
    
    
    
    return 0;
/*    std::cout << "convertPacketToString_Fake" << std::endl;
    
    common::minet::Packet *packet = packet_ptr.get();
    std::cout << "convertPacketToString_Fake 1" << std::endl;
    
 
    
    short cmd_id= packet->cmd_id;
    std::cout << cmd_id << std::endl;
    
    __int64 ret;
    LIBDETOUR_ORIG_GET(&detour_ctx_convertPacketToString, ret, convertPacketToString,packet_ptr , name);
    std::cout << *name << std::endl;
    
    return ret;*/
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
    fprintf(stdout, "Start hooking...\n");
    std::string currentDir = getCurrentDirPath();
    fprintf(stdout, "%s \n" ,currentDir.c_str() );
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

    fprintf(stdout, "File content: %s. \n" , fileContent.c_str());

    Json::Value node;
    Json::Reader reader;
    
    reader.parse(fileContent, node);
    
    get_api_server()=node["api_server"].asString();
    get_api_path()=node["api_path"].asString();
    
    
    int size=node["cmd_name_filter"].size();
    
    std::list<std::string>& cmd_name_filter_list = getCmdNameFilterList();
  
    for(int i = 0; i < size; i++)
    {
        //fprintf(stdout,"%s \n",node["cmd_name_filter"][i].asString().c_str());
        cmd_name_filter_list.push_back(node["cmd_name_filter"][i].asString());
    }
    
    
    void *handle = dlopen(NULL, RTLD_NOW);
    convertPacketToString = dlsym(handle, "_ZN10ProtoUtils21convertPacketToStringESt10shared_ptrIN6common5minet6PacketEERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE");
    getProtoDebugString = dlsym(handle, "_ZNK6common5minet6Packet19getProtoDebugStringB5cxx11Ev");
    DebugString=dlsym(handle, "_ZNK6google8protobuf7Message11DebugStringB5cxx11Ev");
    
    getCmdName=(getCmdName_ptr) dlsym(handle, "_ZN10ProtoUtils10getCmdNameB5cxx11Ej");
    
    getRegionName=(getRegionName_ptr) dlsym(handle, "_ZN10ConfigBase13getRegionNameB5cxx11Ev");
    findService=(findService_ptr) dlsym(handle, "_ZN10ServiceBox11findServiceI17GameserverServiceEEPT_v");
    getConfig=(getConfig_ptr) dlsym(handle, "_ZN17GameserverService9getConfigEv");
    libdetour_init(&detour_ctx_convertPacketToString, convertPacketToString, (void*)convertPacketToString_Fake);
    libdetour_add(&detour_ctx_convertPacketToString);
    
    libdetour_init(&detour_ctx_getProtoDebugString, getProtoDebugString, (void*)getProtoDebugString_Fake);
    libdetour_add(&detour_ctx_getProtoDebugString);
    
    libdetour_init(&detour_ctx_DebugString, DebugString, (void*)DebugString_Fake);
    libdetour_add(&detour_ctx_DebugString);
    
    fprintf(stdout, "Hook done!\n");
}