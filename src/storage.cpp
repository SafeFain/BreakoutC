#include "storage.h"

#include <SDL3/SDL.h>

#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace neon {

static bool isValidColor(const std::string& s) {
  if (s.size() != 7 || s[0] != '#') return false;
  for (size_t i = 1; i < 7; i++) {
    char c = s[i];
    if (!(std::isdigit(static_cast<unsigned char>(c)) ||
          (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
      return false;
  }
  return true;
}

static std::string storagePath() {
  // SDL_GetPrefPath 返回 ~/.local/share/<org>/<app>/ 目录(带尾部斜杠)
  char* base = SDL_GetPrefPath("NeonBreaker", "settings");
  std::string path = base ? std::string(base) + "settings.kv" : "settings.kv";
  SDL_free(base);
  return path;
}

static std::string trim(const std::string& s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

StorageData loadStorage() {
  StorageData d;
  std::ifstream in(storagePath());
  if (!in) return d;
  std::string line;
  while (std::getline(in, line)) {
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = trim(line.substr(0, eq));
    std::string val = trim(line.substr(eq + 1));
    if (key == "highScore") {
      try { int v = std::stoi(val); d.highScore = v > 0 ? v : 0; } catch (...) {}
    } else if (key == "soundEnabled") {
      d.soundEnabled = (val == "true");
    } else if (key == "language") {
      d.language = (val == "en") ? Language::EN : Language::ZH;
    } else if (key == "ballTrail") {
      d.ballTrail = (val == "true");
    } else if (key == "ballColor") {
      if (isValidColor(val)) d.ballColor = val;
    } else if (key == "themeMode") {
      d.themeMode = (val == "light") ? ThemeMode::Light : (val == "dark") ? ThemeMode::Dark : ThemeMode::System;
    } else if (key == "impactFx") {
      d.impactFx = (val == "true");
    }
  }
  return d;
}

void saveStorage(const StorageData& d) {
  char* dir = SDL_GetPrefPath("NeonBreaker", "settings");
  if (!dir) return;
  std::string path = std::string(dir) + "settings.kv";
  SDL_free(dir);
  std::ofstream out(path);
  if (!out) return;
  out << "highScore=" << d.highScore << "\n";
  out << "soundEnabled=" << (d.soundEnabled ? "true" : "false") << "\n";
  out << "language=" << (d.language == Language::EN ? "en" : "zh-CN") << "\n";
  out << "ballTrail=" << (d.ballTrail ? "true" : "false") << "\n";
  out << "ballColor=" << d.ballColor << "\n";
  out << "themeMode=" << (d.themeMode == ThemeMode::Light ? "light" : d.themeMode == ThemeMode::Dark ? "dark" : "system") << "\n";
  out << "impactFx=" << (d.impactFx ? "true" : "false") << "\n";
}

} // namespace neon
