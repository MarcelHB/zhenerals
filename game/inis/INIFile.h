#ifndef H_INI_FILE
#define H_INI_FILE

#include <istream>
#include <string>
#include <vector>

namespace ZH {

class INIFile {
  protected:
    INIFile(std::istream&);
    void advanceStream();
    std::string consumeComment();
    std::string getToken();

    std::istream& stream;
  private:
    std::vector<char> readBuffer;
};

}

#endif
