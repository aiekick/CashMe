#include <Abstract/Base.h>
#include <ctools/cTools.h>

bool parseDescription(const std::string& vDesc,  //
                      std::string& vOutEntity,
                      std::string& vOutOperation,
                      std::string& vOutDescription) {
    bool ret = false;
    if (!vDesc.empty()) {
        vOutDescription = vDesc;
        const auto first_not_space_pos = vOutDescription.find_first_not_of(' ');
        if (first_not_space_pos != std::string::npos) {
            auto space_pos = vOutDescription.find(' ', first_not_space_pos);
            if (space_pos != std::string::npos) {
                vOutOperation = vOutDescription.substr(first_not_space_pos, space_pos - first_not_space_pos);
                ++space_pos;  // inc from ' '
                const auto first_slash_pos = vOutDescription.find('/', space_pos);
                if (first_slash_pos != std::string::npos) {
                    const auto end_space_pos = vOutDescription.rfind(' ', first_slash_pos);
                    if (end_space_pos != std::string::npos) {
                        vOutEntity = vOutDescription.substr(space_pos, end_space_pos - space_pos);
                    } 
                } 
                if (vOutEntity.empty()) {
                    vOutEntity = vOutDescription.substr(space_pos);
                }
            }
        }
        while (ct::replaceString(vOutEntity, "  ", " ")) {
        }
        while (ct::replaceString(vOutOperation, "  ", " ")) {
        }
        while (ct::replaceString(vOutDescription, "  ", " ")) {
        }
        if (vOutDescription.front() == ' ') {
            vOutDescription = vOutDescription.substr(1);
        }
        if (vOutDescription.back() == ' ') {
            vOutDescription = vOutDescription.substr(0, vOutDescription.size() - 1U);
        }
        ret = true;
    }
    return ret;
}

bool Base::init(Cash::PluginBridge* vBridgePtr)  {
    return true;
};

void Base::unit() {
};

std::string Base::getFileExt() const  {
    return "";
}
