#include <Utils/Utils.h>
#include <ctools/cTools.h>

bool parseDescription(const std::string& vDesc,  //
                      std::string& vOutEntity,
                      std::string& vOutOperation,
                      std::string& vOutDescription) {
    bool ret = false;
    if (!vDesc.empty()) {
        const auto& arr = ct::splitStringToVector(vDesc, ' ');

        size_t found_date_pos = arr.size();
        if (arr.size() > 0) {
            // search for date format : 25 03 24
            const auto date_word0 = arr.at(arr.size() - 1);
            if (date_word0.size() == 2U) {
                if (date_word0.find_first_not_of("0123456789") == std::string::npos) {
                    if (arr.size() > 2) {
                        const auto date_word1 = arr.at(arr.size() - 2);
                        if (date_word1.size() == 2U && date_word1.find_first_not_of("0123456789") == std::string::npos) {
                            if (arr.size() > 3) {
                                const auto date_word2 = arr.at(arr.size() - 1);
                                if (date_word2.size() == 2U && date_word2.find_first_not_of("0123456789") == std::string::npos) {
                                    // date format : 25 03 24
                                    found_date_pos = arr.size() - 3;
                                }
                            }
                        }
                    }
                } 
            } else if (date_word0.find_first_not_of("0123456789/") == std::string::npos) {
                if (ct::GetCountOccurence(date_word0, '/') == 2U) {
                    // date format : 25/03/24
                    found_date_pos = arr.size() - 1;
                }
            }

            vOutOperation = arr.at(0);

            vOutEntity.clear();
            for (size_t i = 1U; i < found_date_pos; ++i) {
                if (i > 1U) {
                    vOutEntity += ' ';
                }
                vOutEntity += arr.at(i);
            }

            vOutDescription = vOutOperation + " " + vOutEntity;
        }

        ret = true;
    }
    return ret;
}
