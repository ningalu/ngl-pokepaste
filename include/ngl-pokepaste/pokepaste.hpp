#ifndef NGL_POKEPASTE_POKEPASTE_HPP
#define NGL_POKEPASTE_POKEPASTE_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <compare>
#include <format>
#include <iostream>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ngl {
namespace util {

template <typename T>
[[nodiscard]] auto to_underlying(T value) {
  static_assert(std::is_enum_v<T>);
  return static_cast<std::underlying_type_t<T>>(value);
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
    throw std::out_of_range{
      "String split delimiter is longer than the string being split"
    };
  }
  if (delimiter.size() == str.size()) {
    return {""};
  }
  if (delimiter.empty()) {
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

[[nodiscard]] inline bool contains(const std::string &str, const std::string &find) {
  return split(str, find).size() > 1;
}

[[nodiscard]] inline std::string to_upper(const std::string &str) {
  auto out = str;
  std::ranges::transform(out.begin(), out.end(), out.begin(), [](char c) {
    return static_cast<char>(std::toupper(c));
  });
  return out;
}

[[nodiscard]] inline std::string to_lower(const std::string &str) {
  auto out = str;
  std::ranges::transform(out.begin(), out.end(), out.begin(), [](char c) {
    return static_cast<char>(std::tolower(c));
  });
  return out;
}

} // namespace util

namespace pokepaste {
enum class Gender : uint8_t {
  M,
  F
};
} // namespace pokepaste

[[nodiscard]] inline std::string repr(const ngl::pokepaste::Gender &gender) {
  switch (gender) {
  case ngl::pokepaste::Gender::M:
    return "ngl::pokepaste::Gender::M";
  case ngl::pokepaste::Gender::F:
    return "ngl::pokepaste::Gender::F";
  default:
    return "ngl::pokepaste::Gender::" + std::to_string(util::to_underlying(gender)) + " (invalid Gender value)";
  }
}

[[nodiscard]] inline std::string str(const ngl::pokepaste::Gender &gender) {
  switch (gender) {
  case ngl::pokepaste::Gender::M:
    return "M";
  case ngl::pokepaste::Gender::F:
    return "F";
  default:
    return "Invalid Gender";
  }
}

namespace pokepaste {

struct Pokemon {
  struct Stats {
    constexpr static std::size_t NUM_STATS = 6;
    std::size_t hp = 0, atk = 0, def = 0, spatk = 0, spdef = 0, spd = 0;
    [[nodiscard]] bool operator==(const Stats &) const noexcept = default;
    [[nodiscard]] std::strong_ordering operator<=>(
      const Stats &
    ) const noexcept = default;
  };

  constexpr static std::size_t DEFAULT_HAPPINESS     = 255;
  constexpr static std::size_t DEFAULT_DYNAMAX_LEVEL = 10;
  constexpr static Stats DEFAULT_IVS                 = {31, 31, 31, 31, 31, 31};

  std::optional<std::string> nickname = std::nullopt;
  std::string species;
  std::optional<Gender> gender    = std::nullopt;
  std::optional<std::string> item = std::nullopt;

  // showdown import/export order
  std::string ability;
  std::optional<std::size_t> level;
  bool shiny                = false;
  std::size_t happiness     = DEFAULT_HAPPINESS;
  std::size_t dynamax_level = DEFAULT_DYNAMAX_LEVEL;
  bool gigantamax           = false;
  std::optional<std::string> tera_type;
  Stats evs;
  std::optional<std::string> nature;
  Stats ivs = DEFAULT_IVS;
  std::vector<std::string> moves;

  [[nodiscard]] bool operator==(const Pokemon &) const                  = default;
  [[nodiscard]] std::strong_ordering operator<=>(const Pokemon &) const = default;
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
  [[nodiscard]] bool operator==(
    const SpeciesLineInfo &
  ) const noexcept = default;
  [[nodiscard]] std::strong_ordering operator<=>(
    const SpeciesLineInfo &
  ) const noexcept = default;
};

[[nodiscard]] inline std::string encode_string_line(const std::string &line, const std::string &prefix) {
  return std::format("{} {}", util::trim(prefix), util::trim(line));
}

[[nodiscard]] inline std::string decode_string_line(const std::string &line, const std::string &prefix) {
  assert(util::starts_with(line, prefix));
  return util::trim(line.substr(prefix.size()));
}

[[nodiscard]] inline std::string encode_number_line(int number, const std::string &prefix) {
  return encode_string_line(std::to_string(number), prefix);
}

[[nodiscard]] inline int decode_number_line(const std::string &line, const std::string &prefix) {
  assert(util::starts_with(line, prefix));
  const auto str = decode_string_line(line, prefix);
  return std::stoi(str);
}

[[nodiscard]] inline std::string encode_bool_line(bool value, const std::string &prefix) {
  return encode_string_line(value ? "Yes" : "No", prefix);
}

[[nodiscard]] inline bool decode_bool_line(const std::string &line, const std::string &prefix) {
  assert(util::starts_with(line, prefix));
  const auto str = util::to_lower(decode_string_line(line, prefix));
  if (str == "yes") {
    return true;
  }

  if (str == "no") {
    return false;
  }

  throw std::runtime_error{R"(Boolean data payload must be "Yes" or "No")"};
}

[[nodiscard]] inline std::string encode_stat_line(
  const Pokemon::Stats &stats,
  const std::string &prefix,
  const Pokemon::Stats &baseline = {}
) {
  std::vector<std::string> parts;
  if (stats.hp != baseline.hp) {
    parts.push_back(std::format("{} HP", std::to_string(stats.hp)));
  }
  if (stats.atk != baseline.atk) {
    parts.push_back(std::format("{} Atk", std::to_string(stats.atk)));
  }
  if (stats.def != baseline.def) {
    parts.push_back(std::format("{} Def", std::to_string(stats.def)));
  }
  if (stats.spatk != baseline.spatk) {
    parts.push_back(std::format("{} SpA", std::to_string(stats.spatk)));
  }
  if (stats.spdef != baseline.spdef) {
    parts.push_back(std::format("{} SpD", std::to_string(stats.spdef)));
  }
  if (stats.spd != baseline.spd) {
    parts.push_back(std::format("{} Spe", std::to_string(stats.spd)));
  }
  return encode_string_line(util::join(parts, " / "), prefix);
}

[[nodiscard]] inline Pokemon::Stats decode_stat_line(
  const std::string &line, const std::string &prefix, const Pokemon::Stats &default_stats = {}
) {
  const auto body    = decode_string_line(line, prefix);
  const auto entries = util::split(body, "/");
  if (entries.empty()) {
    throw std::runtime_error{
      "Pokemon stat line must contain at least one value"
    };
  }
  if (entries.size() > Pokemon::Stats::NUM_STATS) {
    throw std::runtime_error{"Pokemon may not specify more than 6 stat values"};
  }

  auto stats                                        = default_stats;
  const std::unordered_set<std::string> valid_stats = {"hp", "atk", "def", "spa", "spd", "spe"};
  std::unordered_map<std::string, std::size_t> stat_history;
  for (const auto &raw_entry : entries) {
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
      throw std::runtime_error{
        "Pokemon may not specify multiple values for a single stat"
      };
    }
    stat_history[stat] = static_cast<std::size_t>(value);
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
      throw std::runtime_error{"Unreachable"};
    }
  }

  return stats;
}

[[nodiscard]] inline std::string encode_name_line(const SpeciesLineInfo &info) {
  std::string out;
  if (info.nickname.has_value()) {
    out.append(info.nickname.value());
    out.append(" (");
    out.append(info.species);
    out.append(")");
  } else {
    out.append(info.species);
  }

  if (info.gender.has_value()) {
    switch (info.gender.value()) {
    case Gender::M:
      out.append(" (M)");
      break;
    case Gender::F:
      out.append(" (F)");
      break;
    default:
      throw std::runtime_error{"Unreachable"};
    }
  }

  if (info.item.has_value()) {
    out.append(" @ ");
    out.append(info.item.value());
  }

  return out;
}

[[nodiscard]] inline SpeciesLineInfo decode_name_line(const std::string &line) {
  SpeciesLineInfo out;

  // Line has at least one open paren; could be gender, nickname + species, or random value
  if (util::contains(line, " (")) {
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
        // There is no item marker, so the final gender marker must be interpreted as the canonical
        // gender marker and the rest of the string must contain the species and maybe a nickname
        out.gender                  = util::contains(line, "(M)") ? Gender::M : Gender::F;
        const auto gender_substring = std::string{out.gender.value() == Gender::M ? "(M)" : "(F)"};
        auto parts                  = util::split(line, gender_substring);
        parts.pop_back();
        species_and_nickname = util::join(parts, gender_substring);
      }
    } else {
      // An lparen is present but a gender marker isn't; try to
      // consume a potential item term and use the rest of the string as a
      // species + nickname
      auto names_and_maybe_item = util::split(line, " @ ");
      if (names_and_maybe_item.size() > 1) {
        out.item = util::trim(names_and_maybe_item.back());
        names_and_maybe_item.pop_back();
      }
      species_and_nickname = util::join(names_and_maybe_item, " @ ");
    }
    // If the species and possible nickname string extracted contains an
    // lparen there might still be a nickname here
    species_and_nickname = util::trim(species_and_nickname);
    if (util::contains(species_and_nickname, " (")) {
      // If any matching rparen exists then this string has a
      // nickname and a species
      const auto lparen = species_and_nickname.find(std::string{" ("});
      const auto rparen = species_and_nickname.rfind(std::string{")"});
      if ((rparen != std::string::npos) && (rparen > lparen)) {
        if (rparen != (species_and_nickname.size() - 1)) {
          throw std::runtime_error{"Malformed nickname and species data"};
        }
        auto parts = util::split(species_and_nickname, " (");
        assert(parts.size() > 1);
        out.nickname = util::trim(parts.front());
        parts.erase(parts.begin());
        out.species = util::join(parts, " (");
        out.species = util::trim(out.species.substr(0, out.species.size() - 1)); // Remove last rparen
      } else {
        out.species = util::trim(species_and_nickname);
      }
    } else {
      out.species = util::trim(species_and_nickname);
    }

  } else {
    // No lparens means there can only be a species name and item
    auto species_and_maybe_item = util::split(line, " @ ");
    if (species_and_maybe_item.size() > 1) {
      out.item = util::trim(species_and_maybe_item.back());
      species_and_maybe_item.pop_back();
    }
    out.species = util::trim(util::join(species_and_maybe_item, " @ "));
  }

  return out;
}

[[nodiscard]] inline std::string encode_ability_line(const std::string &ability) {
  return encode_string_line(ability, "Ability:");
}

[[nodiscard]] inline std::string decode_ability_line(const std::string &line) {
  auto value = decode_string_line(line, "Ability:");
  if (value.empty()) {
    throw std::runtime_error{"Pokemon Ability line must contain a value"};
  }
  return value;
}

[[nodiscard]] inline std::string encode_level_line(std::size_t level) {
  assert(level <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  return encode_number_line(static_cast<int>(level), "Level:");
}

[[nodiscard]] inline std::size_t decode_level_line(const std::string &line) {
  const auto value = decode_number_line(line, "Level:");
  if (value < 1) {
    throw std::runtime_error{"Pokemon Level cannot be less than 0"};
  }
  return static_cast<std::size_t>(value);
}

[[nodiscard]] inline std::string encode_shiny_line(bool shiny) {
  return encode_bool_line(shiny, "Shiny:");
}

[[nodiscard]] inline bool decode_shiny_line(const std::string &line) {
  try {
    return decode_bool_line(line, "Shiny:");
  } catch ([[maybe_unused]] const std::runtime_error &e) {
    throw std::runtime_error{R"(Pokemon Shiny line data must be "Yes" or "No")"};
  }
}

[[nodiscard]] inline std::string encode_happiness_line(std::size_t happiness) {
  assert(happiness <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  return encode_number_line(static_cast<int>(happiness), "Happiness:");
}

[[nodiscard]] inline std::size_t decode_happiness_line(
  const std::string &line
) {
  const auto value = decode_number_line(line, "Happiness:");
  if (value < 1) {
    throw std::runtime_error{"Pokemon Happiness cannot be less than 0"};
  }
  return static_cast<std::size_t>(value);
}

[[nodiscard]] inline std::string encode_dynamax_level_line(std::size_t dynamax_level) {
  assert(dynamax_level <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  return encode_number_line(static_cast<int>(dynamax_level), "Dynamax Level:");
}

[[nodiscard]] inline std::size_t decode_dynamax_level_line(
  const std::string &line
) {
  const auto value = decode_number_line(line, "Dynamax Level:");
  if (value < 1) {
    throw std::runtime_error{"Pokemon Dynamax Level cannot be less than 0"};
  }
  return static_cast<std::size_t>(value);
}

[[nodiscard]] inline std::string encode_gigantamax_line(bool gmax) {
  return encode_bool_line(gmax, "Gigantamax:");
}

[[nodiscard]] inline bool decode_gigantamax_line(const std::string &line) {
  try {
    return decode_bool_line(line, "Gigantamax:");
  } catch ([[maybe_unused]] const std::runtime_error &e) {
    throw std::runtime_error{R"(Pokemon Gigantamax line data must be "Yes" or "No")"};
  }
}

[[nodiscard]] inline std::string encode_tera_type_line(const std::string &tera_type) {
  return encode_string_line(tera_type, "Tera Type:");
}

[[nodiscard]] inline std::string decode_tera_type_line(
  const std::string &line
) {
  auto value = decode_string_line(line, "Tera Type:");
  if (value.empty()) {
    throw std::runtime_error{
      "Pokemon's Tera Type line must contain a value"
    };
  }
  return value;
}

[[nodiscard]] inline std::string encode_evs_line(const Pokemon::Stats &evs) {
  return encode_stat_line(evs, "EVs: ");
}

[[nodiscard]] inline Pokemon::Stats decode_evs_line(const std::string &line) {
  return decode_stat_line(line, "EVs:");
}

[[nodiscard]] inline std::string encode_nature_line(const std::string &nature) {
  return std::format("{} Nature", nature);
}

[[nodiscard]] inline std::string decode_nature_line(const std::string &line) {
  auto upto  = line.rfind("Nature");
  auto value = util::trim(line.substr(0, upto));
  if (value.empty()) {
    throw std::runtime_error{"Pokemon Nature line must contain a value"};
  }
  return value;
}

[[nodiscard]] inline std::string encode_ivs_line(const Pokemon::Stats &ivs) {
  return encode_stat_line(ivs, "IVs: ", Pokemon::DEFAULT_IVS);
}

[[nodiscard]] inline Pokemon::Stats decode_ivs_line(const std::string &line) {
  return decode_stat_line(line, "IVs:", Pokemon::DEFAULT_IVS);
}

[[nodiscard]] inline std::string encode_move_line(const std::string &move) {
  return encode_string_line(move, "-");
}

[[nodiscard]] inline std::string decode_move_line(const std::string &line) {
  auto value = decode_string_line(line, "-");
  if (value.empty()) {
    throw std::runtime_error{"Pokemon Move line must contain a value"};
  }
  return value;
}

} // namespace detail

[[nodiscard]] inline std::string encode_pokemon(const Pokemon &pokemon) {
  std::vector<std::string> parts;
  parts.push_back(
    detail::encode_name_line(
      detail::SpeciesLineInfo{
        pokemon.nickname,
        pokemon.species,
        pokemon.gender,
        pokemon.item
      }
    )
  );
  parts.push_back(detail::encode_ability_line(pokemon.ability));
  if (pokemon.level.has_value()) {
    parts.push_back(detail::encode_level_line(pokemon.level.value()));
  }
  if (pokemon.shiny) {
    parts.push_back(detail::encode_shiny_line(pokemon.shiny));
  }
  if (pokemon.happiness != Pokemon::DEFAULT_HAPPINESS) {
    parts.push_back(detail::encode_happiness_line(pokemon.happiness));
  }
  if (pokemon.dynamax_level != Pokemon::DEFAULT_DYNAMAX_LEVEL) {
    parts.push_back(detail::encode_dynamax_level_line(pokemon.dynamax_level));
  }
  if (pokemon.gigantamax) {
    parts.push_back(detail::encode_gigantamax_line(pokemon.gigantamax));
  }
  if (pokemon.tera_type.has_value()) {
    parts.push_back(detail::encode_tera_type_line(pokemon.tera_type.value()));
  }
  if (pokemon.evs != Pokemon::Stats{0, 0, 0, 0, 0, 0}) {
    parts.push_back(detail::encode_evs_line(pokemon.evs));
  }
  if (pokemon.nature.has_value()) {
    parts.push_back(detail::encode_nature_line(pokemon.nature.value()));
  }
  if (pokemon.ivs != Pokemon::DEFAULT_IVS) {
    parts.push_back(detail::encode_ivs_line(pokemon.ivs));
  }
  for (const auto &move : pokemon.moves) {
    parts.push_back(detail::encode_move_line(move));
  }
  return util::join(parts, "\n");
}

[[nodiscard]] inline Pokemon decode_pokemon(const std::string &data) {
  Pokemon out;
  const auto fixed_newlines = util::join(util::split(data, "\r\n"), "\n");
  auto parts                = util::split(util::trim(fixed_newlines), "\n");
  if (util::trim(parts.back()).empty()) {
    parts.pop_back();
  }
  if (parts.size() <= 1) {
    throw std::runtime_error{"Not enough lines in Pokemon data"};
  }
  const auto &[nickname, species, gender, item] = detail::decode_name_line(parts.front());
  out.nickname                                  = nickname;
  out.species                                   = species;
  out.gender                                    = gender;
  out.item                                      = item;

  std::unordered_set<std::string> found;
  const auto body = std::span{parts}.subspan(1, parts.size() - 1);
  for (const auto &raw_line : body) {
    const auto line = util::trim(raw_line);
    std::optional<std::string> key;
    if (util::starts_with(line, "Ability:")) {
      key         = "Ability";
      out.ability = detail::decode_ability_line(line);
    } else if (util::starts_with(line, "Level:")) {
      key       = "Level";
      out.level = detail::decode_level_line(line);
    } else if (util::starts_with(line, "Shiny:")) {
      key       = "Shiny";
      out.shiny = detail::decode_shiny_line(line);
    } else if (util::starts_with(line, "Happiness:")) {
      key           = "Happiness";
      out.happiness = detail::decode_happiness_line(line);
    } else if (util::starts_with(line, "Dynamax Level:")) {
      key               = "Dynamax Level";
      out.dynamax_level = detail::decode_dynamax_level_line(line);
    } else if (util::starts_with(line, "Gigantamax:")) {
      key            = "Gigantamax";
      out.gigantamax = detail::decode_gigantamax_line(line);
    } else if (util::starts_with(line, "Tera Type:")) {
      key           = "Tera Type";
      out.tera_type = detail::decode_tera_type_line(line);
    } else if (util::starts_with(line, "EVs:")) {
      key     = "EVs";
      out.evs = detail::decode_evs_line(line);
    } else if (util::ends_with(line, "Nature")) {
      key        = "Nature";
      out.nature = detail::decode_nature_line(line);
    } else if (util::starts_with(line, "IVs:")) {
      key     = "IVs";
      out.ivs = detail::decode_ivs_line(line);
    } else if (util::starts_with(line, "-")) {
      out.moves.push_back(detail::decode_move_line(line));
    } else {
      throw std::runtime_error{"Unknown line in Pokemon data"};
    }
    if (key.has_value()) {
      if (found.contains(key.value())) {
        throw std::runtime_error{"Duplicate line detected"};
      }
      found.insert(key.value());
    }
  }

  if (!found.contains("Ability")) {
    throw std::runtime_error{"Pokemon requires Ability data"};
  }

  return out;
}

[[nodiscard]] inline std::string encode_pokepaste(const PokePaste &paste) {
  std::string out;
  for (const auto &pokemon : paste) {
    out.append(std::format("{}\n\n", util::trim(encode_pokemon(pokemon))));
  }
  return util::trim(out);
}

[[nodiscard]] inline PokePaste decode_pokepaste(const std::string &paste) {
  PokePaste out;
  const auto fixed_newlines = util::join(util::split(paste, "\r\n"), "\n");
  const auto split_newlines = util::split(fixed_newlines, "\n\n");
  std::vector<std::string> team;
  for (const auto &pokemon : split_newlines) {
    const auto trimmed = util::trim(pokemon);
    if (!trimmed.empty()) {
      team.push_back(trimmed);
    }
  }
  for (const auto &pokemon : team) {
    out.push_back(decode_pokemon(util::trim(pokemon)));
  }

  return out;
}

} // namespace pokepaste

[[nodiscard]] inline std::string repr(const ngl::pokepaste::detail::SpeciesLineInfo &data) {
  std::string out = "ngl::pokepaste::detail::SpeciesLineInfo {\n";
  out.append(
    std::format(
      "\tNickname: {}\n",
      data.nickname.has_value() ? data.nickname.value() : std::string{"std::nullopt"}
    )
  );
  out.append(
    std::format(
      "\tSpecies: {}\n",
      data.species
    )
  );
  out.append(
    std::format(
      "\tGender: {}\n",
      data.gender.has_value() ? repr(data.gender.value()) : std::string{"std::nullopt"}
    )
  );
  out.append(
    std::format(
      "\tItem: {}\n",
      data.item.has_value() ? data.item.value() : std::string{"std::nullopt"}
    )
  );
  out.append("}");
  return out;
}

[[nodiscard]] inline std::string str(const pokepaste::detail::SpeciesLineInfo &data) {
  return ngl::pokepaste::detail::encode_name_line(data);
}

[[nodiscard]] inline std::string repr(const pokepaste::Pokemon &pokemon) {
  std::vector<std::string> parts;
  parts.emplace_back("ngl::pokepaste::Pokemon {");
  parts.push_back(std::format("\tNickname: \"{}\"", pokemon.nickname.has_value() ? pokemon.nickname.value() : std::string{"std::nullopt"}));
  parts.push_back(std::format("\tSpecies: \"{}\"", pokemon.species));
  parts.push_back(std::format("\tGender: {}", pokemon.gender.has_value() ? repr(pokemon.gender.value()) : std::string{"std::nullopt"}));
  parts.push_back(std::format("\tItem: \"{}\"", pokemon.item.has_value() ? pokemon.item.value() : std::string{"std::nullopt"}));
  parts.push_back(std::format("\tAbility: \"{}\"", pokemon.ability));
  parts.push_back(std::format("\tLevel: {}", pokemon.level.has_value() ? std::to_string(pokemon.level.value()) : std::string{"std::nullopt"}));
  parts.push_back(std::format("\tShiny: {}", pokemon.shiny ? "True" : "False"));
  parts.push_back(std::format("\tHappiness: {}", pokemon.happiness));
  parts.push_back(std::format("\tDynamax Level: {}", pokemon.dynamax_level));
  parts.push_back(std::format("\tGigantamax: {}", pokemon.gigantamax ? "True" : "False"));
  parts.push_back(std::format("\tTera Type: \"{}\"", pokemon.tera_type.has_value() ? pokemon.tera_type.value() : std::string{"std::nullopt"}));
  parts.push_back(std::format("\t{}", pokepaste::detail::encode_evs_line(pokemon.evs)));
  parts.push_back(std::format("\tNature: \"{}\"", pokemon.nature.has_value() ? pokemon.nature.value() : std::string{"std::nullopt"}));
  parts.push_back(std::format("\t{}", pokepaste::detail::encode_ivs_line(pokemon.ivs)));
  for (std::size_t i = 0; i < pokemon.moves.size(); i++) {
    parts.push_back(std::format("\tMove {}: \"{}\"", i, pokemon.moves[i]));
  }
  parts.emplace_back("}");
  return util::trim(util::join(parts, "\n"));
}

[[nodiscard]] inline std::string str(const pokepaste::Pokemon &pokemon) {
  return pokepaste::encode_pokemon(pokemon);
}

[[nodiscard]] inline std::string repr(const pokepaste::PokePaste &paste) {
  std::vector<std::string> out;
  for (const auto &pokemon : paste) {
    out.push_back(util::trim(repr(pokemon)));
  }
  const auto body = util::join(out, ",\n");
  return "ngl::pokepaste::PokePaste {\n" + body + "\n}";
}

[[nodiscard]] inline std::string str(const pokepaste::PokePaste &paste) {
  return encode_pokepaste(paste);
}

} // namespace ngl

inline std::ostream &operator<<(std::ostream &os, const ngl::pokepaste::detail::SpeciesLineInfo &data) {
  os << ngl::repr(data);
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const ngl::pokepaste::Pokemon &data) {
  os << ngl::repr(data);
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const ngl::pokepaste::PokePaste &data) {
  os << ngl::repr(data);
  return os;
}

#endif