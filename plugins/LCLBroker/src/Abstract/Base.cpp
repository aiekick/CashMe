#include <Abstract/Base.h>

bool Base::init(Cash::PluginBridge* vBridgePtr)  {
    return true;
};

void Base::unit() {
};

std::string Base::getFileExt() const  {
    return "";
}
