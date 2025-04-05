#ifndef H_CSF_FILE
#define H_CSF_FILE

#include <istream>
#include <optional>
#include <string>
#include <unordered_map>

namespace ZH {

class CSFFile {
  public:
    struct StringEntry {
      std::u16string string;
      std::optional<std::string> soundFile;
    };

    using StringMap = std::unordered_map<std::string, StringEntry>;

    CSFFile(std::istream&);
    StringMap getStrings();
  private:
    std::istream& stream;
};

}

#endif
