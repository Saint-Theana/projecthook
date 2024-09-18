//获取函数直接调用
DO_APP_FUNC("_ZN10ProtoUtils10getCmdNameB5cxx11Ej", std::string * , getCmdName, (uint32_t cmd_id));
DO_APP_FUNC("_ZN10ConfigBase13getRegionNameB5cxx11Ev", std::string * , getRegionName, (Config *config));
DO_APP_FUNC("_ZN10ServiceBox11findServiceI17GameserverServiceEEPT_v",GameserverService *,findService,());
DO_APP_FUNC("_ZN17GameserverService9getConfigEv",std::shared_ptr<Config>,getConfig,(GameserverService *service));
DO_APP_FUNC("_ZNK6common5minet6Packet19getProtoDebugStringB5cxx11Ev",std::string *,getProtoDebugString,(std::string *out,common::minet::Packet *packet));
DO_APP_FUNC("_ZN6common5minet6Packet8getProtoEv",google::protobuf::Message *,getProto,(std::shared_ptr<google::protobuf::Message> *cmd_id,common::minet::Packet *packet));

//将函数保存在别的位置，通过跳转来调用被hook的函数
STORE_APP_FUNC("_ZN10ProtoUtils21convertPacketToStringESt10shared_ptrIN6common5minet6PacketEERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",__int64,convertPacketToString,(std::shared_ptr<common::minet::Packet> packet_ptr, std::string *name));
STORE_APP_FUNC("_ZNK6google8protobuf7Message11DebugStringB5cxx11Ev",std::string *,DebugString,(std::string *retstr,google::protobuf::Message *message));
STORE_APP_FUNC("_ZN10ConfigBase13getRegionNameB5cxx11Ev", std::string * , getRegionNameEX, (Config *config));
STORE_APP_FUNC("_ZN17GameserverService9getConfigEv",std::shared_ptr<Config>,getConfigEX,(GameserverService *service));
STORE_APP_FUNC("_ZN10ServiceBox11findServiceI17GameserverServiceEEPT_v",GameserverService *,findServiceEX,());

