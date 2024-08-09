#include <map>
#include <string>
#include "format.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include "common/milog.h"
#include "common/minet.h"
#include <dlfcn.h>
#include "capstone/capstone.h"
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
#include "app-types.h"
#include "PFishHook.h"
#include "Zydis/Zydis.h"
#define __int64 long long


#define STORE_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) RETURN_T (*NAME) PARAMS
#define DO_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) RETURN_T (*NAME) PARAMS
#include "functions.h"
#undef DO_APP_FUNC
#undef STORE_APP_FUNC
#define PAGE_START(x,pagesize)	((x) &~ ((pagesize) - 1))
#define CODE_WRITE				PROT_READ | PROT_WRITE | PROT_EXEC
#define CODE_READ_ONLY			PROT_READ | PROT_EXEC


void  *tempAddr = (void  *)0x2a52000;


int get_asm_len(intptr_t target)
{
    csh handle;
    cs_insn* insn;
    size_t count;
    uint8_t code[30] = {0};
    int rv;
 
    memcpy((void*)code, (void*)target, 30);
    if(cs_open(CS_ARCH_X86, CS_MODE_64, &handle))
    {
        printf("Error: cs_open\n");
        return -1;
    }
 
    count = cs_disasm(handle, code, 30, 0, 0, &insn);
    if(count)
    {
        for(size_t i = 0; i < count; i++)
        {
            if (insn[i].address >= 12)
            {
                rv = insn[i].address;
                break;
            }
        }
        cs_free(insn, count);
    }
    else
    {
        printf("Error: cs_disasm\n");
        return -1;
    }
    cs_close(&handle);
    return rv;
}


void make_phony_function(void **function,void *target_function){
    intptr_t target_addr=(intptr_t)target_function;
    intptr_t page_start = target_addr & 0xfffffffff000;
    mprotect((void*)page_start, 0x1000, PROT_READ|PROT_EXEC);
    int asm_len = get_asm_len(target_addr);
    
    std::cout<<asm_len<<std::endl;
    if(asm_len <= 0)
    {
        std::cout<<"Error: get_asm_len\n"<<std::endl;
    }
    char* temp_func = (char*)mmap(tempAddr, 4096, PROT_WRITE|PROT_EXEC|PROT_READ, MAP_ANON|MAP_PRIVATE, -1, 0);

    
    memcpy((void*)temp_func, (void*)target_addr, asm_len);
    intptr_t y = (intptr_t)target_addr + asm_len;
    //构造push&ret跳转，填入目标地址
    /*char jmp_code[14] = {0x68,y&0xff,(y&0xff00)>>8,(y&0xff0000)>>16,(y&0xff000000)>>24,
        0xC7,0x44,0x24,0x04,(y&0xff00000000)>>32,(y&0xff0000000000)>>40,(y&0xff000000000000)>>48,
        (y&0xff00000000000000)>>56,0xC3};
    
    memcpy((void*)(temp_func+asm_len), (void*)jmp_code, 14);
    */
    
    fprintf(stderr, "temp_func 0x%llx\n",temp_func);
    
    fprintf(stderr, "target_addr 0x%llx\n",target_addr);
    
    
    uint32_t relativeAddr = y - ((intptr_t)temp_func+asm_len+5);
    fprintf(stderr, "relativeAddr 0x%llx\n",relativeAddr);
    
    uint8_t jmp_code[5] = {0xe9,0x00,0x00,0x00,0x00};
    memcpy(jmp_code + 1, &relativeAddr, sizeof(uint32_t));
    
    memcpy((void*)(temp_func+asm_len), (void*)jmp_code, 5);
    
    mprotect((void*)page_start, 0x1000, PROT_READ | PROT_EXEC);
    *function = (void*)temp_func;
    



}
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

std::string *getRegionName_Fake(Config *config){
    INFO("getRegionName_Fake \n");
    std::string *name= getRegionNameEX(config);
    INFO("getRegionName_Fake result %s\n",(*name).c_str());
    return name;
}


std::string *DebugString_Fake(std::string *retstr,google::protobuf::Message *message){
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true; // 设置为 true 启用缩进
    google::protobuf::util::MessageToJsonString(*message,retstr,options);
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
    getProtoDebugString(&bodystr,packet);
    
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
        INFO("cmd: %s. found in cmd_name_filter_list\n" , cmd_name.c_str());
        INFO("api_path: %s. \n" , get_api_path().c_str());
        INFO("api_path: %s. \n" , get_api_server().c_str());
        
        httplib::Client cli(get_api_server());
        httplib::Result res = cli.Post(str_format("%s?region=%s&uid=%d&cmd_name=%s",get_api_path().c_str(),region_name.c_str(),uid,cmd_name.c_str()),bodystr, "text/plain");
        
    } 
    return 0;
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

    INFO("File content: %s. \n" , fileContent.c_str());

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
    
    #define DO_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) NAME = (RETURN_T (*) PARAMS) dlsym(handle, OFFSET);
    #define STORE_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) 
    #include "functions.h"
    #undef DO_APP_FUNC
    #undef STORE_APP_FUNC
    
    #define DO_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) 
    #define STORE_APP_FUNC(OFFSET, RETURN_T, NAME, PARAMS) FakeIt(dlsym(handle, OFFSET),(void**)&NAME);
    #include "functions.h"
    #undef DO_APP_FUNC
    #undef STORE_APP_FUNC
    
    
    GameserverService *service =findService();
    Config *config=getConfig(service).get();
    INFO("Hook done!\n");
}


// ---

GameserverService * findServiceE(){
    std::cout<<"AA"<<std::endl;
    std::cout<<"BB"<<std::endl;
    std::cout<<"DD"<<std::endl;
    std::cout<<"SS"<<std::endl;
    return nullptr;
}

#include <iostream>
#include <capstone/capstone.h>




void disassembleFunction(void* func, size_t size) {
    csh handle;
    cs_insn* insn;
    size_t count;

    // 初始化 Capstone 反汇编器
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        std::cerr << "Failed to initialize Capstone" << std::endl;
        return;
    }

    // 反汇编函数的机器代码
    count = cs_disasm(handle, (uint8_t*)func, size, (uint64_t)func, 0, &insn);
    if (count > 0) {
        for (size_t i = 0; i < count; i++) {
            std::cout << "0x" << std::hex << insn[i].address << ":\t"
                      << insn[i].mnemonic << "\t"
                      << insn[i].op_str << std::endl;
        }

        // 释放 Capstone 使用的内存
        cs_free(insn, count);
    } else {
        std::cerr << "Failed to disassemble function" << std::endl;
    }

    cs_close(&handle);
}


extern "C" std::map<std::string,std::string> getFuncMap() {
   std::map<std::string,std::string> hookFuncMap ={
    {
      "_ZN10ProtoUtils21convertPacketToStringESt10shared_ptrIN6common5minet6PacketEERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",
      "_Z26convertPacketToString_FakeSt10shared_ptrIN6common5minet6PacketEEPNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE"
    },
    {
      "_ZNK6google8protobuf7Message11DebugStringB5cxx11Ev",
      "_Z16DebugString_FakePNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEPN6google8protobuf7MessageE"
    }/*,
    {
      "_ZN10ConfigBase13getRegionNameB5cxx11Ev",
      "_Z18getRegionName_FakeB5cxx11P6Config"
    }*/
  };
  return hookFuncMap;
}

// ---


