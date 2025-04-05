#include <limits>
#include <optional>
#include <string_view>
#include <utility>

#include "WNDFile.h"

namespace ZH {

template<typename T>
std::pair<T, bool> stringToNumber(const std::string& s) {
  try {
    if constexpr (std::numeric_limits<T>::is_signed) {
      return {std::stol(s.c_str(), nullptr), true};
    } else {
      return {std::stoul(s.c_str(), nullptr), true};
    }
  } catch (std::invalid_argument) {
    return {0, false};
  } catch (std::out_of_range) {
    return {0, false};
  }
}

template<typename T>
static std::pair<T, bool> stringToNumber(const std::string_view v) {
  return stringToNumber<T>(std::string {v});
}

static bool parseBool(const std::string& v) {
  return v != "0";
}

template<typename T>
static BitField<T> parseEnumBitField(const std::string& v, std::function<T(const std::string_view)> getter) {
  auto currentBegin = v.cbegin();
  auto it = v.cbegin();
  BitField<T> field;

  for (; it != v.cend(); ++it) {
    if (*it == '+') {
      auto v = getter(std::string_view {currentBegin, it});
      field |= v;
      currentBegin = it + 1;
    }
  }

  if (currentBegin != v.cend()) {
    auto v = getter(std::string_view {currentBegin, it});
    field |= v;
  }

  return field;
}

static std::optional<uint32_t> parseNonNegative(const std::string& v) {
  if (v[0] == '-') {
    return {};
  } else {
    auto conversion = stringToNumber<uint32_t>(v);
    return std::make_optional(conversion.first);
  }
}

template<typename T, size_t N>
static std::array<typename T::underlying_type, N> parseNTuple(const std::string& v) {
  std::array<typename T::underlying_type, N> values = {0};
  auto currentBegin = v.cbegin();
  auto it = v.cbegin();

  size_t i = 0;
  for (; it != v.cend() && i < N; ++it) {
    if (*it == ' ') {
      auto conversion = stringToNumber<typename T::underlying_type>(std::string_view {currentBegin, it});
      values[i] = conversion.first;
      while (it != v.cend() && *it == ' ') {
        it++;
      }

      ++i;
      currentBegin = it;
    }
  }

  if (it != v.cbegin()) {
    auto conversion = stringToNumber<typename T::underlying_type>(std::string_view {currentBegin, it});
    values[i] = conversion.first;
  }

  return values;
}

static Color parseColor(const std::string& s) {
  auto v = parseNTuple<Color, 4>(s);

  return {v[0], v[1], v[2], v[3]};
}

static std::optional<std::string> parseImage(const std::string& v) {
  if (v == "NoImage") {
    return {};
  } else {
    return {v};
  }
}

static std::optional<std::string> parseString(const std::string& v) {
  if (v == "[None]" || v == "[NONE]") {
    return {};
  } else {
    return {v};
  }
}

static ApplierMap<GUI::WND::DrawData::Entry> DrawDataKVMap = {
  { "IMAGE", [](GUI::WND::DrawData::Entry& b, const std::string& v) { b.image = parseImage(v); }},
  { "COLOR", [](GUI::WND::DrawData::Entry& b, const std::string& v) { b.color = parseColor(v); }},
  { "BORDERCOLOR", [](GUI::WND::DrawData::Entry& b, const std::string& v) { b.borderColor = parseColor(v); }}
};

static ApplierMap<GUI::WND::Font::Builder> FontKVMap = {
  { "NAME", [](GUI::WND::Font::Builder& b, const std::string& v) { b.name = parseString(v); }},
  { "SIZE", [](GUI::WND::Font::Builder& b, const std::string& v) { b.size = parseNonNegative(v); }},
  { "BOLD", [](GUI::WND::Font::Builder& b, const std::string& v) { b.bold = parseBool(v); }}
};

static ApplierMap<GUI::WND::Layout::Builder> LayoutBlockKVMap = {
  { "LAYOUTINIT", [](GUI::WND::Layout::Builder& b, const std::string& v) { b.initFunction = parseString(v); }},
  { "LAYOUTUPDATE", [](GUI::WND::Layout::Builder& b, const std::string& v) { b.updateFunction = parseString(v); }},
  { "LAYOUTSHUTDOWN", [](GUI::WND::Layout::Builder& b, const std::string& v) { b.shutdownFunction = parseString(v); }}
};

static ApplierMap<GUI::WND::ScreenRect::Builder> ScreenRectKVMap = {
  { "UPPERLEFT", [](GUI::WND::ScreenRect::Builder& b, const std::string& v) { b.upperLeft = parseNTuple<Point, 2>(v); }},
  { "BOTTOMRIGHT", [](GUI::WND::ScreenRect::Builder& b, const std::string& v) { b.bottomRight = parseNTuple<Point, 2>(v); }},
  { "CREATIONRESOLUTION", [](GUI::WND::ScreenRect::Builder& b, const std::string& v) { b.resolution = parseNTuple<Size, 2>(v); }}
};

static ApplierMap<GUI::WND::TextColor::Builder> TextColorKVMap = {
  { "DISABLED", [](GUI::WND::TextColor::Builder& b, const std::string& v) { b.disabled = parseColor(v); }},
  { "DISABLEDBORDER", [](GUI::WND::TextColor::Builder& b, const std::string& v) { b.disabledBorder = parseColor(v); }},
  { "ENABLED", [](GUI::WND::TextColor::Builder& b, const std::string& v) { b.enabled = parseColor(v); }},
  { "ENABLEDBORDER", [](GUI::WND::TextColor::Builder& b, const std::string& v) { b.enabledBorder = parseColor(v); }},
  { "HILITE", [](GUI::WND::TextColor::Builder& b, const std::string& v) { b.highlight = parseColor(v); }},
  { "HILITEBORDER", [](GUI::WND::TextColor::Builder& b, const std::string& v) { b.highlightBorder = parseColor(v); }},
};

static ApplierMap<GUI::WND::Window::Builder> WindowKVMap = {
  { "DRAWCALLBACK", [](GUI::WND::Window::Builder& b, const std::string& v) { b.drawCallback = parseString(v); }},
  { "HEADERTEMPLATE", [](GUI::WND::Window::Builder& b, const std::string& v) { b.headerTemplate = parseString(v); }},
  { "INPUTCALLBACK", [](GUI::WND::Window::Builder& b, const std::string& v) { b.inputCallback = parseString(v); }},
  { "NAME", [](GUI::WND::Window::Builder& b, const std::string& v) { b.name = parseString(v); }},
  { "STATUS", [](GUI::WND::Window::Builder& b, const std::string& v) { b.status = parseEnumBitField<GUI::WND::Window::Status>(v, GUI::WND::Window::Builder::getStatusByString); }},
  { "STYLE", [](GUI::WND::Window::Builder& b, const std::string& v) { b.style = GUI::WND::Window::Builder::getStyleByString(v); }},
  { "SYSTEMCALLBACK", [](GUI::WND::Window::Builder& b, const std::string& v) { b.systemCallback = parseString(v); }},
  { "TEXT", [](GUI::WND::Window::Builder& b, const std::string& v) { b.textRef = parseString(v); }},
  { "TOOLTIPCALLBACK", [](GUI::WND::Window::Builder& b, const std::string& v) { b.tooltipCallback = parseString(v); }},
  { "TOOLTIPDELAY", [](GUI::WND::Window::Builder& b, const std::string& v) { b.tooltipDelay = parseNonNegative(v); }},
  { "WINDOWTYPE", [](GUI::WND::Window::Builder& b, const std::string& v) { b.type = GUI::WND::Window::Builder::getTypeByString(v); }},
};


WNDFile::WNDFile(std::istream& instream) : stream(instream) {
  readBuffer.reserve(128);
}

std::shared_ptr<GUI::WND::WindowAndLayout> WNDFile::parse() {
  TRACY(ZoneScoped);
  std::shared_ptr<GUI::WND::Layout> layout;

  advanceStream();
  auto token = getToken();
  if (token != "FILE_VERSION") {
    return {};
  }

  token = getKeyValue();
  if (token.empty()) {
    return {};
  }

  if (token == "2") {
    layout = parseLayout();
  }

  auto window = parseWindow();
  if (!window) {
    return {};
  }

  return std::make_shared<GUI::WND::WindowAndLayout>(std::move(window), std::move(layout));
}

void WNDFile::advanceStream() {
  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r') {
      stream.get();
    } else {
      break;
    }
  } while (!stream.eof());
}

bool WNDFile::consumeAssignment() {
  advanceStream();
  return getToken() == "=";
}

std::string WNDFile::getAttributeValue() {
  readBuffer.clear();
  auto peek = stream.peek();

  if (peek == '"') {
    stream.get();
    auto rest = getKeyValueUntilQuote();
    rest = rest.substr(0, rest.size() - 1);
    return rest;
  }

  while (!stream.eof()) {
    peek = stream.peek();
    if (peek == ',' || peek == ';') {
      break;
    } else {
      auto c = stream.get();
      readBuffer.push_back(c);
    }
  }

  return {readBuffer.cbegin(), readBuffer.cend()};
}

std::string WNDFile::getKeyValueUntilQuote() {
  readBuffer.clear();

  while (!stream.eof()) {
    auto c = stream.get();
    readBuffer.push_back(c);
    if (c == '"') {
      break;
    }
  }

  return {readBuffer.cbegin(), readBuffer.cend()};
}

// Also consumes, final ';'
std::string WNDFile::getKeyValue() {
  if (!consumeAssignment()) {
    return {};
  }

  advanceStream();
  auto value = getToken();
  // "ab c"
  if (value[0] == '"' && *(value.cend() - 1) != '"') {
    auto rest = getKeyValueUntilQuote();
    value.append(rest);
    // strip quotes
    value = value.substr(1, value.size() - 2);
  } else if (value[0] == '"' && *(value.cend() - 1) == '"') {
    value = value.substr(1, value.size() - 2);
  } else {
    advanceStream();
  }

  auto token = getToken();
  // We have parsed too much, broken file
  if (token != ";") {
    return {};
  }

  return value;
}

std::string WNDFile::getToken() {
  readBuffer.clear();

  // get first whatever it is
  auto c = stream.get();
  readBuffer.push_back(c);

  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r' || peek == ';' || peek == ',' || peek == '=' || peek == ':') {
      break;
    } else {
      auto c = stream.get();
      readBuffer.push_back(c);
    }
  } while (!stream.eof());

  return {readBuffer.cbegin(), readBuffer.cend()};
}

std::shared_ptr<GUI::WND::DrawData> WNDFile::parseDrawData() {
  GUI::WND::DrawData::Entries entries;

  advanceStream();
  auto token = getToken();

  for (uint8_t i = 0; i < 9; ++i) {
    auto& entry = entries[i];

    for (uint8_t j = 0; j < 3; ++j) {
      auto key = token;

      advanceStream();
      token = getToken();
      if (token != ":") {
        return {};
      }

      advanceStream();
      auto value = getAttributeValue();

      applyValueByKey(DrawDataKVMap, entry, key, value);

      advanceStream();
      token = getToken();
      if (token == ",") {
        advanceStream();
        token = getToken();
      }
    }
  }

  if (token != ";") {
    return {};
  }

  return std::make_shared<GUI::WND::DrawData>(std::move(entries));
}

std::shared_ptr<GUI::WND::Layout> WNDFile::parseLayout() {
  advanceStream();
  auto token = getToken();
  if (token != "STARTLAYOUTBLOCK") {
    return {};
  }

  GUI::WND::Layout::Builder builder;

  advanceStream();
  token = getToken();
  while (token != "ENDLAYOUTBLOCK" && !stream.eof()) {
    advanceStream();
    auto value = getKeyValue();
    applyValueByKey(LayoutBlockKVMap, builder, token, value);

    advanceStream();
    token = getToken();
  }

  return std::make_shared<GUI::WND::Layout>(std::move(builder));
}

std::shared_ptr<GUI::WND::Window> WNDFile::parseWindow() {
  advanceStream();
  auto token = getToken();

  if (token != "WINDOW") {
    return {};
  }

  GUI::WND::Window::Builder builder;

  advanceStream();
  token = getToken();
  while (token != "END" && !stream.eof()) {
    if (token == "CHILD") {
      while (token != "ENDALLCHILDREN" && !stream.eof()) {
        auto childWindow = parseWindow();
        if (childWindow) {
          builder.children.emplace_back(std::move(childWindow));
        }

        advanceStream();
        token = getToken();
      }
    } else if (token == "FONT") {
      if (consumeAssignment()) {
        builder.font = parseAttributeBlock<GUI::WND::Font>(FontKVMap);
      }
    } else if (token == "SCREENRECT") {
      if (consumeAssignment()) {
        builder.screenRect = parseAttributeBlock<GUI::WND::ScreenRect>(ScreenRectKVMap);
      }
    } else if (token == "TEXTCOLOR") {
      if (consumeAssignment()) {
        builder.textColor = parseAttributeBlock<GUI::WND::TextColor>(TextColorKVMap);
      }
    } else if (token == "ENABLEDDRAWDATA" || token == "DISABLEDDRAWDATA" || token == "HILITEDRAWDATA") {
      if (consumeAssignment()) {
        auto drawData = parseDrawData();
        if (drawData) {
          if (token == "ENABLEDDRAWDATA") {
            builder.enabled = std::move(drawData);
          } else if (token == "DISABLEDDRAWDATA") {
            builder.disabled = std::move(drawData);
          } else if (token == "HILITEDRAWDATA") {
            builder.highlight = std::move(drawData);
          }
        }
      }
    } else {
      auto value = getKeyValue();
      applyValueByKey(WindowKVMap, builder, token, value);
    }

    advanceStream();
    token = getToken();
  }

  return std::make_shared<GUI::WND::Window>(std::move(builder));
}

}
