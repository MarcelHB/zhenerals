#ifndef H_WND_FILE
#define H_WND_FILE

#include <functional>
#include <istream>
#include <map>
#include <vector>

#include "../common.h"
#include "../GUI/wnd/WindowAndLayout.h"

namespace ZH {

template<typename T>
using Applier = std::function<void(T&, const std::string&)>;

template<typename T>
using ApplierMap = const std::map<std::string, Applier<T>>;

template <typename T>
bool applyValueByKey(ApplierMap<T>& map, T& obj, const std::string& key, const std::string& value) {
  auto it = map.find(key);
  if (it == map.cend()) {
    return false;
  }

  it->second(obj, value);

  return true;
}

class WNDFile {
  public:
    WNDFile(std::istream& instream);

    std::shared_ptr<GUI::WND::WindowAndLayout> parse();
  private:
    std::istream& stream;
    std::vector<char> readBuffer;

    void advanceStream();
    bool consumeAssignment();
    std::string getToken();
    std::string getAttributeValue();
    std::string getKeyValue();
    std::string getKeyValueUntilQuote();

    template<typename B>
    bool parseAttributes(B& b, ApplierMap<B>& map) {
      advanceStream();
      auto token = getToken();

      while (token != ";") {
        auto key = token;
        advanceStream();
        token = getToken();

        if (token != ":") {
          return false;
        }

        advanceStream();
        token = getAttributeValue();
        applyValueByKey(map, b, key, token);

        advanceStream();
        token = getToken();

        if (token == ",") {
          advanceStream();
          token = getToken();
        }
      }

      return true;
    }

    template<typename T>
    std::shared_ptr<T> parseAttributeBlock(const ApplierMap<typename T::Builder>& map) {
      typename T::Builder builder;
      if (parseAttributes(builder, map)) {
        return std::make_shared<T>(std::move(builder));
      } else {
        return {};
      }
    }

    std::shared_ptr<GUI::WND::DrawData> parseDrawData();
    std::shared_ptr<GUI::WND::Layout> parseLayout();
    std::shared_ptr<GUI::WND::Window> parseWindow();
};

}

#endif
