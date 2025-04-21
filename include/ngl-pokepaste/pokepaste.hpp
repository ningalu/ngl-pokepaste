#ifndef NGL_POKEPASTE_POKEPASTE_HPP
#define NGL_POKEPASTE_POKEPASTE_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <compare>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ngl {
namespace pokepaste {

namespace util {
[[nodiscard]] inline std::string trim(const std::string &str) {
  const auto begin = str.find_first_not_of(" \t\r\n");

  if (begin == std::string::npos) {
    return "";
  }

  const auto end = str.find_last_not_of(" \t\r\n");
  return str.substr(begin, end - begin + 1);
}

[[nodiscard]] inline bool starts_with(const std::string &str, const std::string &prefix) {
  return (str.size() >= prefix.size()) && (std::equal(prefix.begin(), prefix.end(), str.begin()));
}

[[nodiscard]] inline bool ends_with(const std::string &str, const std::string &suffix) {
  return (str.size() >= suffix.size()) && (std::equal(suffix.rbegin(), suffix.rend(), str.rbegin()));
}

[[nodiscard]] inline std::string join(const std::vector<std::string> &strs, const std::string &joiner = "") {
  std::string out;
  for (auto it = strs.begin(); it != strs.end(); it++) {

    out.append(*it);
    if (it != (strs.end() - 1)) {
      out.append(joiner);
    }
  }
  return out;
}

[[nodiscard]] inline std::vector<std::string> split(const std::string &str, const std::string &delimiter) {
  if (delimiter.size() > str.size()) {
    throw std::out_of_range{"String split delimiter is longer than the string being split"};
  }
  if (delimiter.size() == str.size()) {
    return {""};
  }
  if (delimiter.size() == 0) {
    return {str};
  }
  std::vector<std::string> out;
  std::size_t current = 0;
  std::size_t last    = 0;
  while ((current + delimiter.size()) <= str.size()) {
    if ((str[current] == delimiter[0]) && str.substr(current, delimiter.size()) == delimiter) {

      out.push_back(str.substr(last, current - last));
      current += delimiter.size();
      last = current;
    } else {
      current++;
    }
  }
  out.push_back(str.substr(last, str.size() - last));
  return out;
}

[[nodiscard]] inline bool contains(const std::string &str, const std::string &find) {
  return split(str, find).size() > 1;
}

[[nodiscard]] inline std::string to_upper(const std::string &str) {
  auto out = str;
  std::transform(out.begin(), out.end(), out.begin(), std::toupper);
  return out;
}

[[nodiscard]] inline std::string to_lower(const std::string &str) {
  auto out = str;
  std::transform(out.begin(), out.end(), out.begin(), std::tolower);
  return out;
}

template <std::size_t n>
[[nodiscard]] inline std::string trimmed_substr_size(const std::string &str, const char(prefix)[n]) {
  return trim(str.substr(std::size(prefix)));
}

} // namespace util

enum class Gender {
  M,
  F
};

struct Pokemon {
  struct Stats {
    std::size_t hp = 0, atk = 0, def = 0, spatk = 0, spdef = 0, spd = 0;
    [[nodiscard]] inline bool operator==(const Stats &) const noexcept                  = default;
    [[nodiscard]] inline std::strong_ordering operator<=>(const Stats &) const noexcept = default;
  };

  std::optional<std::string> nickname;
  std::string species;
  std::optional<Gender> gender;
  std::optional<std::string> item;

  // showdown import/export order
  std::string ability;
  std::size_t level;
  bool shiny;
  std::size_t happiness;
  std::size_t dynamax_level;
  std::optional<bool> gigantamax;
  std::optional<std::string> tera_type;
  Stats evs;
  std::optional<std::string> nature;
  Stats ivs;
  std::vector<std::string> moves;
};

using PokePaste = std::vector<Pokemon>;

class domain_bound_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

namespace detail {

struct SpeciesLineInfo {
  std::optional<std::string> nickname;
  std::string species;
  std::optional<Gender> gender;
  std::optional<std::string> item;
  [[nodiscard]] inline bool operator==(const SpeciesLineInfo &) const noexcept                  = default;
  [[nodiscard]] inline std::strong_ordering operator<=>(const SpeciesLineInfo &) const noexcept = default;
};

[[nodiscard]] inline std::string
decode_string_line(const std::string &line, const std::string &prefix) {
  assert(util::starts_with(line, prefix));
  return util::trim(line.substr(prefix.size()));
}

[[nodiscard]] inline int decode_number_line(const std::string &line, const std::string &prefix) {
  assert(util::starts_with(line, prefix));
  const auto str = decode_string_line(line, prefix);
  return std::stoi(str);
}

[[nodiscard]] inline bool decode_bool_line(const std::string &line, const std::string &prefix) {
  assert(util::starts_with(line, prefix));
  const auto str = util::to_lower(decode_string_line(line, prefix));
  if (str == "yes") {
    return true;
  } else if (str == "no") {
    return false;
  } else {
    throw std::runtime_error{"Boolean data payload must be \"Yes\" or \"No\""};
  }
}

[[nodiscard]] inline Pokemon::Stats decode_stat_line(const std::string &line, const std::string &prefix, const Pokemon::Stats &default_stats = {}) {
  const auto body    = decode_string_line(line, prefix);
  const auto entries = util::split(body, "/");
  if (entries.size() == 0) {
    throw std::runtime_error{"Pokemon stat line must contain at least one value"};
  }
  if (entries.size() > 6) {
    throw std::runtime_error{"Pokemon may not specify more than 6 stat values"};
  }

  auto stats                                        = default_stats;
  const std::unordered_set<std::string> valid_stats = {"hp", "atk", "def", "spa", "spd", "spe"};
  std::unordered_map<std::string, std::size_t> stat_history;
  for (const auto raw_entry : entries) {
    const auto parts = util::split(util::trim(raw_entry), " ");
    if (parts.size() != 2) {
      throw std::runtime_error{"Stat entry data is malformed"};
    }
    const auto value = std::stoi(parts.front());
    if (value < 0) {
      throw std::runtime_error{"Stat value cannot be less than 0"};
    }
    const auto stat = util::to_lower(util::trim(parts.back()));
    if (!valid_stats.contains(stat)) {
      throw std::runtime_error{"Invalid stat name"};
    }
    if (stat_history.contains(stat)) {
      throw std::runtime_error{"Pokemon may not specify multiple values for a single stat"};
    } else {
      stat_history[stat] = static_cast<std::size_t>(value);
    }
  }

  for (const auto &[key, value] : stat_history) {
    if (key == "hp") {
      stats.hp = value;
    } else if (key == "atk") {
      stats.atk = value;
    } else if (key == "def") {
      stats.def = value;
    } else if (key == "spa") {
      stats.spatk = value;
    } else if (key == "spd") {
      stats.spdef = value;
    } else if (key == "spe") {
      stats.spd = value;
    } else {
      throw std::exception{"Unreachable"};
    }
  }

  return stats;
}

[[nodiscard]] inline SpeciesLineInfo decode_name_line(const std::string &line) {
  SpeciesLineInfo out;

  // Line has at least one open paren; could be gender, nickname + species, or random value
  if (util::contains(line, "(")) {
    std::string species_and_nickname;
    // The final occurrence of an (M) or (F) must be a gender indicator
    if (util::contains(line, "(M)") || util::contains(line, "(F)")) {
      // The final combination of a potential gender marker and item marker MUST be interpreted as such
      if (util::contains(line, "(M) @ ")) {
        out.gender = Gender::M;
        auto parts = util::split(line, "(M) @ ");
        out.item   = util::trim(parts.back());
        parts.pop_back();
        species_and_nickname = util::join(parts, "(F) @ ");
      } else if (util::contains(line, "(F) @ ")) {
        out.gender = Gender::F;
        auto parts = util::split(line, "(F) @ ");
        out.item   = util::trim(parts.back());
        parts.pop_back();
        species_and_nickname = util::join(parts, "(F) @ ");
      } else {
        // There is no item marker, so the final gender marker must be interpreted as the gender
        // marker and the rest of the string must contain the species and maybe a nickname
        out.gender                  = util::contains(line, "(M)") ? Gender::M : Gender::F;
        const auto gender_substring = out.gender.value() == Gender::M ? "(M)" : "(F)";
        auto parts                  = util::split(line, gender_substring);
        parts.pop_back();
        species_and_nickname = util::join(parts, gender_substring);
      }
    } else {
      // An open paren is present but a gender marker isn't; try to consume a potential
      // item term and use the rest of the string as a species + nickname
      auto names_and_maybe_item = util::split(line, " @ ");
      if (names_and_maybe_item.size() > 1) {
        out.item = util::trim(names_and_maybe_item.back());
        names_and_maybe_item.pop_back();
      }
      species_and_nickname = util::trim(util::join(names_and_maybe_item, " @ ")); // TODO: Nickname
    }
    // If the species and possible nickname string extracted contains an open
    if (util::contains(species_and_nickname, "(")) {
      out.species = util::trim(species_and_nickname); // TODO: Nickname
    } else {
      out.species = util::trim(species_and_nickname);
    }

  } else {
    // No open parens means there can only be a species name and item
    auto species_and_maybe_item = util::split(line, " @ ");
    if (species_and_maybe_item.size() > 1) {
      out.item = util::trim(species_and_maybe_item.back());
      species_and_maybe_item.pop_back();
    }
    out.species = util::trim(util::join(species_and_maybe_item, " @ "));
  }

  return out;
}

[[nodiscard]] inline std::string decode_ability_line(const std::string &line) {
  const auto value = decode_string_line(line, "Ability:");
  if (value.size() == 0) {
    throw std::runtime_error{"Pokemon Ability line must contain a value"};
  }
  return value;
}

[[nodiscard]] inline std::size_t decode_level_line(const std::string &line) {
  const auto value = decode_number_line(line, "Level:");
  if (value < 1) {
    throw std::runtime_error{"Pokemon Level cannot be less than 0"};
  }
  return static_cast<std::size_t>(value);
}

[[nodiscard]] inline bool decode_shiny_line(const std::string &line) {
  try {
    return decode_bool_line(line, "Shiny:");
  } catch ([[maybe_unused]] const std::runtime_error &e) {
    throw std::runtime_error{"Pokemon Shiny line data must be \"Yes\" or \"No\""};
  }
}

[[nodiscard]] inline std::size_t decode_happiness_line(const std::string &line) {
  const auto value = decode_number_line(line, "Happiness:");
  if (value < 1) {
    throw std::runtime_error{"Pokemon Happiness cannot be less than 0"};
  }
  return static_cast<std::size_t>(value);
}

[[nodiscard]] inline std::size_t decode_dynamax_level_line(const std::string &line) {
  const auto value = decode_number_line(line, "Dynamax Level:");
  if (value < 1) {
    throw std::runtime_error{"Pokemon Dynamax Level cannot be less than 0"};
  }
  return static_cast<std::size_t>(value);
}

[[nodiscard]] inline bool decode_gigantamax_line(const std::string &line) {
  try {
    return decode_bool_line(line, "Gigantamax:");
  } catch ([[maybe_unused]] const std::runtime_error &e) {
    throw std::runtime_error{"Pokemon Gigantamax line data must be \"Yes\" or \"No\""};
  }
}

[[nodiscard]] inline std::string decode_tera_type_line(const std::string &line) {
  const auto value = decode_string_line(line, "Tera Type:");
  if (value.size() == 0) {
    throw std::runtime_error{"Pokemon's Tera Type line must contain a value"};
  }
  return value;
}

[[nodiscard]] inline Pokemon::Stats decode_evs_line(const std::string &line) {
  return decode_stat_line(line, "EVs:");
}

[[nodiscard]] inline std::string decode_nature_line(const std::string &line) {
  const auto value = decode_string_line(line, "Nature:");
  if (value.size() == 0) {
    throw std::runtime_error{"Pokemon Nature line must contain a value"};
  }
  return value;
}

[[nodiscard]] inline Pokemon::Stats decode_ivs_line(const std::string &line) {
  return decode_stat_line(line, "IVs:", Pokemon::Stats{31, 31, 31, 31, 31, 31});
}

[[nodiscard]] inline std::string decode_move_line(const std::string &line) {
  const auto value = decode_string_line(line, "-");
  if (value.size() == 0) {
    throw std::runtime_error{"Pokemon Move line must contain a value"};
  }
  return value;
}

} // namespace detail
} // namespace pokepaste

} // namespace ngl

#endif