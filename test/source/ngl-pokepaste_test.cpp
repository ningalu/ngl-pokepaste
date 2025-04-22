
#include <cassert>
#include <cstring>
#include <iostream>
#include <optional>
#include <source_location>

#include "ngl-pokepaste/pokepaste.hpp"

static bool verbose = false;

#define CHECK_EQ(lhs, rhs)                                        \
  if (lhs == rhs) {                                               \
    if (verbose) {                                                \
      std::cout << "Test success at line "                        \
                << std::source_location::current().line() << "\n" \
                << lhs << " == " << rhs << "\n";                  \
    }                                                             \
  } else {                                                        \
    std::cout << "Test failure at line "                          \
              << std::source_location::current().line() << "\n"   \
              << lhs << " != " << rhs << "\n";                    \
  }

std::ostream &operator<<(std::ostream &os, [[maybe_unused]] const std::nullopt_t &data) {
  os << "std::nullopt";
  return os;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::optional<T> &data) {
  os << (data.has_value() ? data.value() : "std::nullopt");
  return os;
}

auto main(int argc, const char **argv) -> int {
  if ((argc > 1) && (std::string{argv[1]} == "-v")) {
    verbose = true;
  } else {
    verbose = false;
  }

  // ngl::util
  {
    {
      const auto trim1 = ngl::util::trim(" abc ");
      assert((trim1 == "abc"));
    }

    {
      const auto ends_value    = "ends with";
      const auto ends_result   = ngl::util::ends_with(ends_value, "with");
      const auto ends_expected = true;
      assert((ends_result == ends_expected));
    }

    {
      const auto contains_value    = "contains";
      const auto contains_result   = ngl::util::contains(contains_value, "ta");
      const auto contains_expected = true;
      assert((contains_result == contains_expected));
    }

    {
      const auto contains_value    = "contains";
      const auto contains_result   = ngl::util::contains(contains_value, "z");
      const auto contains_expected = false;
      assert((contains_result == contains_expected));
    }

    {
      const auto join_value = std::vector{
        std::string{"a"}, std::string{"b"}, std::string{"c"}
      };
      const auto join_result   = ngl::util::join(join_value);
      const auto join_expected = std::string{"abc"};
      assert((join_result == join_expected));
    }

    {
      const auto join_value = std::vector{
        std::string{"a"}, std::string{"b"}, std::string{"c"}
      };
      const auto join_result   = ngl::util::join(join_value, " ");
      const auto join_expected = std::string{"a b c"};
      assert((join_result == join_expected));
    }

    {
      const auto join_value    = std::vector{std::string{"a"}};
      const auto join_result   = ngl::util::join(join_value, "1");
      const auto join_expected = std::string{"a"};
      assert((join_result == join_expected));
    }

    {
      const auto split_value    = ngl::util::split("a b c d e f g", " ");
      const auto split_expected = std::vector<std::string>{"a", "b", "c", "d", "e", "f", "g"};
      assert((split_value == split_expected));
    }

    {
      const auto split_value    = ngl::util::split("abc", " ");
      const auto split_expected = std::vector<std::string>{"abc"};
      assert((split_value == split_expected));
    }

    {
      const auto split_value    = ngl::util::split("abbcccbba", "c");
      const auto split_expected = std::vector<std::string>{"abb", "", "", "bba"};
      assert((split_value == split_expected));
      const auto join_value    = ngl::util::join(split_value, "c");
      const auto join_expected = std::string{"abbcccbba"};
      assert((join_value == join_expected));
    }

    {
      const auto upper_value    = std::string{"AbCdEfG"};
      const auto upper_result   = ngl::util::to_upper(upper_value);
      const auto upper_expected = std::string{"ABCDEFG"};
      assert((upper_result == upper_expected));
    }

    {
      const auto lower_value    = std::string{"AbCdEfG"};
      const auto lower_result   = ngl::util::to_lower(lower_value);
      const auto lower_expected = std::string{"abcdefg"};
      assert((lower_result == lower_expected));
    }
  }

  // ngl::pokepaste::detail
  {
    {
      const auto name_value    = std::string{"Species"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Species", std::nullopt, std::nullopt
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Sp@cies"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Sp@cies", std::nullopt, std::nullopt
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Species @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Species", std::nullopt, "Item"
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Sp@cies @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Sp@cies", std::nullopt, "Item"
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Species (M) @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Species", ngl::pokepaste::Gender::M, "Item"
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Sp@cies (M) @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Sp@cies", ngl::pokepaste::Gender::M, "Item"
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Species (F) @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Species", ngl::pokepaste::Gender::F, "Item"
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Sp@cies (F) @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        std::nullopt, "Sp@cies", ngl::pokepaste::Gender::F, "Item"
      };
      CHECK_EQ(name_result, name_expected);
    }

    {
      const auto name_value    = std::string{"Nickname (?) @ Item"};
      const auto name_result   = ngl::pokepaste::detail::decode_name_line(name_value);
      const auto name_expected = ngl::pokepaste::detail::SpeciesLineInfo{
        "Nickname", "?", std::nullopt, "Item"
      };
      CHECK_EQ(name_result, name_expected);
      CHECK_EQ(name_result.nickname, name_expected.nickname);
      CHECK_EQ(name_result.species, name_expected.species);
      // CHECK_EQ(name_result.nickname, name_expected.nickname);
      CHECK_EQ(name_result.item, name_expected.item);
    }

    {
      const auto ability_value = std::string{"dummy ability"};
      const auto ability       = ngl::pokepaste::detail::decode_ability_line(
        "Ability: " + ability_value
      );
      assert((ability == ability_value));
    }

    {
      const auto ability_value    = std::string{" trimmable dummy ability "};
      const auto ability_result   = ngl::pokepaste::detail::decode_ability_line("Ability:" + ability_value);
      const auto ability_expected = ngl::util::trim(ability_value);
      assert((ability_result == ability_expected));
    }

    {
      const auto level_value  = std::size_t{15};
      const auto level_result = ngl::pokepaste::detail::decode_level_line(
        "Level:" + std::to_string(level_value)
      );
      const auto level_expected = level_value;
      assert((level_result == level_expected));
    }

    {
      const auto level_value = -1;
      try {
        (void)ngl::pokepaste::detail::decode_level_line(
          "Level:" + std::to_string(level_value)
        );
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto shiny_value  = std::string{"Yes"};
      const auto shiny_result = ngl::pokepaste::detail::decode_shiny_line(
        "Shiny:" + shiny_value
      );
      const auto shiny_expected = true;
      assert((shiny_result == shiny_expected));
    }

    {
      const auto shiny_value  = std::string{"No"};
      const auto shiny_result = ngl::pokepaste::detail::decode_shiny_line(
        "Shiny:" + shiny_value
      );
      const auto shiny_expected = false;
      assert((shiny_result == shiny_expected));
    }

    {
      const auto shiny_value  = std::string{"yes"};
      const auto shiny_result = ngl::pokepaste::detail::decode_shiny_line(
        "Shiny:" + shiny_value
      );
      const auto shiny_expected = true;
      assert((shiny_result == shiny_expected));
    }

    {
      const auto shiny_value = std::string{"invalid"};
      try {
        (void)ngl::pokepaste::detail::decode_shiny_line("Shiny:" + shiny_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto happy_value  = std::size_t{15};
      const auto happy_result = ngl::pokepaste::detail::decode_happiness_line(
        "Happiness:" + std::to_string(happy_value)
      );
      const auto happy_expected = happy_value;
      assert((happy_result == happy_expected));
    }

    {
      const auto dmax_value  = std::size_t{15};
      const auto dmax_result = ngl::pokepaste::detail::decode_dynamax_level_line(
        "Dynamax Level:" + std::to_string(dmax_value)
      );
      const auto dmax_expected = dmax_value;
      assert((dmax_result == dmax_expected));
    }

    {
      const auto gmax_value    = std::string{"Yes"};
      const auto gmax_result   = ngl::pokepaste::detail::decode_gigantamax_line("Gigantamax:" + gmax_value);
      const auto gmax_expected = true;
      assert((gmax_result == gmax_expected));
    }

    {
      const auto gmax_value    = std::string{"No"};
      const auto gmax_result   = ngl::pokepaste::detail::decode_gigantamax_line("Gigantamax:" + gmax_value);
      const auto gmax_expected = false;
      assert((gmax_result == gmax_expected));
    }

    {
      const auto gmax_value    = std::string{"yes"};
      const auto gmax_result   = ngl::pokepaste::detail::decode_gigantamax_line("Gigantamax:" + gmax_value);
      const auto gmax_expected = true;
      assert((gmax_result == gmax_expected));
    }

    {
      const auto gmax_value = std::string{"invalid"};
      try {
        (void)ngl::pokepaste::detail::decode_gigantamax_line(
          "Gigantamax:" + gmax_value
        );
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto tera_value = std::string{"fire"};
      const auto tera1      = ngl::pokepaste::detail::decode_tera_type_line(
        "Tera Type:" + tera_value
      );
      assert((tera1 == tera_value));
    }

    {
      const auto tera_value    = std::string{" water "};
      const auto tera_result   = ngl::pokepaste::detail::decode_tera_type_line("Tera Type: " + tera_value);
      const auto tera_expected = ngl::util::trim(tera_value);
      assert((tera_result == tera_expected));
    }

    {
      const auto ev_value    = std::string{"1 HP / 2 Atk / 3 Def / 4 SpA / 5 SpD / 6 Spe"};
      const auto ev_result   = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
      const auto ev_expected = ngl::pokepaste::Pokemon::Stats{1, 2, 3, 4, 5, 6};
      assert((ev_result == ev_expected));
    }

    {
      const auto ev_value    = std::string{"1 HP / 2 Atk"};
      const auto ev_result   = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
      const auto ev_expected = ngl::pokepaste::Pokemon::Stats{1, 2, 0, 0, 0, 0};
      assert((ev_result == ev_expected));
    }

    {
      const auto ev_value    = std::string{"1 HP "};
      const auto ev_result   = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
      const auto ev_expected = ngl::pokepaste::Pokemon::Stats{1, 0, 0, 0, 0, 0};
      assert((ev_result == ev_expected));
    }

    {
      const auto ev_value = std::string{"1 Sp "};
      try {
        const auto ev_result = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto ev_value = std::string{"1 HP / 1 HP / 1 HP / 1 HP / 1 HP / 1 HP / 1 HP"};
      try {
        const auto ev_result = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto ev_value = std::string{"1 HP / 1 HP "};
      try {
        const auto ev_result = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto ev_value = std::string{"1 HP / -1 Atk "};
      try {
        const auto ev_result = ngl::pokepaste::detail::decode_evs_line("EVs:" + ev_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto nature_value = std::string{"dummy nature"};
      const auto nature       = ngl::pokepaste::detail::decode_nature_line(
        "Nature: " + nature_value
      );
      assert((nature == nature_value));
    }

    {
      const auto nature_value    = std::string{" trimmable dummy nature "};
      const auto nature_result   = ngl::pokepaste::detail::decode_nature_line("Nature:" + nature_value);
      const auto nature_expected = ngl::util::trim(nature_value);
      assert((nature_result == nature_expected));
    }

    {
      const auto iv_value    = std::string{"1 HP / 2 Atk / 3 Def / 4 SpA / 5 SpD / 6 Spe"};
      const auto iv_result   = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
      const auto iv_expected = ngl::pokepaste::Pokemon::Stats{1, 2, 3, 4, 5, 6};
      assert((iv_result == iv_expected));
    }

    {
      const auto iv_value    = std::string{"1 HP / 2 Atk"};
      const auto iv_result   = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
      const auto iv_expected = ngl::pokepaste::Pokemon::Stats{1, 2, 31, 31, 31, 31};
      assert((iv_result == iv_expected));
    }

    {
      const auto iv_value    = std::string{"1 HP "};
      const auto iv_result   = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
      const auto iv_expected = ngl::pokepaste::Pokemon::Stats{1, 31, 31, 31, 31, 31};
      assert((iv_result == iv_expected));
    }

    {
      const auto iv_value = std::string{"1 Sp "};
      try {
        const auto iv_result = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto iv_value = std::string{"1 HP / 1 HP / 1 HP / 1 HP / 1 HP / 1 HP / 1 HP"};
      try {
        const auto iv_result = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto iv_value = std::string{"1 HP / 1 HP "};
      try {
        const auto iv_result = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto iv_value = std::string{"1 HP / -1 Atk "};
      try {
        const auto iv_result = ngl::pokepaste::detail::decode_ivs_line("IVs:" + iv_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto move_value = std::string{"dummy move"};
      const auto move       = ngl::pokepaste::detail::decode_move_line("-" + move_value);
      assert((move == move_value));
    }

    {
      const auto move_value    = std::string{" trimmable dummy move "};
      const auto move_result   = ngl::pokepaste::detail::decode_move_line("-" + move_value);
      const auto move_expected = ngl::util::trim(move_value);
      assert((move_result == move_expected));
    }

    {
      const auto move_value = std::string{""};
      try {
        const auto iv_result = ngl::pokepaste::detail::decode_move_line("-" + move_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }
  }

  // ngl::pokepaste
  {
    {
      const auto pokemon_value =
        "Species\n"
        "Ability: Ability\n";

      const auto pokemon_result   = ngl::pokepaste::decode_pokemon(pokemon_value);
      const auto pokemon_expected = ngl::pokepaste::Pokemon{
        std::nullopt,
        "Species",
        std::nullopt,
        std::nullopt,
        "Ability",
        std::nullopt,
        false,
        255,
        10,
        false,
        std::nullopt,
        ngl::pokepaste::Pokemon::Stats{0, 0, 0, 0, 0, 0},
        std::nullopt,
        ngl::pokepaste::Pokemon::Stats{31, 31, 31, 31, 31, 31},
        std::vector<std::string>{}
      };
      CHECK_EQ(pokemon_result, pokemon_expected);
    }

    {
      const auto pokemon_value =
        "Nickname (Species) (M) @ Item\n"
        "Ability: Ability\n"
        "Level: 50\n"
        "Shiny: Yes\n"
        "Happiness: 73\n"
        "Dynamax Level: 4\n"
        "Gigantamax: Yes\n"
        "Tera Type: Type\n"
        "EVs: 6 HP / 5 Atk / 4 Def / 3 SpA / 2 SpD / 1 Spe\n"
        "Nature: Nature\n"
        "IVs: 1 HP / 2 Atk / 3 Def / 4 SpA / 5 SpD / 6 Spe\n"
        "- Attack 1\n"
        "- Attack 2\n"
        "- Attack 3\n"
        "- Attack 4\n";

      const auto pokemon_result   = ngl::pokepaste::decode_pokemon(pokemon_value);
      const auto pokemon_expected = ngl::pokepaste::Pokemon{
        "Nickname",
        "Species",
        ngl::pokepaste::Gender::M,
        "Item",
        "Ability",
        std::size_t{50},
        true,
        std::size_t{73},
        std::size_t{4},
        true,
        "Type",
        ngl::pokepaste::Pokemon::Stats{6, 5, 4, 3, 2, 1},
        "Nature",
        ngl::pokepaste::Pokemon::Stats{1, 2, 3, 4, 5, 6},
        std::vector{
          std::string{"Attack 1"},
          std::string{"Attack 2"},
          std::string{"Attack 3"},
          std::string{"Attack 4"}
        }
      };
      CHECK_EQ(pokemon_result, pokemon_expected);
    }

    {
      const auto pokemon_value =
        "Species\n";

      try {
        (void)ngl::pokepaste::decode_pokemon(pokemon_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto pokemon_value =
        "Nickname (Species) (M) @ Item\n"
        "Level: 50\n"
        "Ability: Ability\n"
        "Happiness: 73\n"
        "Shiny: Yes\n"
        "Gigantamax: Yes\n"
        "- Attack 3\n"
        "- Attack 1\n"
        "Tera Type: Type\n"
        "EVs: 6 HP / 5 Atk / 4 Def / 3 SpA / 2 SpD / 1 Spe\n"
        "Nature: Nature\n"
        "- Attack 2\n"
        "Dynamax Level: 4\n"
        "IVs: 1 HP / 2 Atk / 3 Def / 4 SpA / 5 SpD / 6 Spe\n"
        "- Attack 4\n";

      const auto pokemon_result   = ngl::pokepaste::decode_pokemon(pokemon_value);
      const auto pokemon_expected = ngl::pokepaste::Pokemon{
        "Nickname",
        "Species",
        ngl::pokepaste::Gender::M,
        "Item",
        "Ability",
        std::size_t{50},
        true,
        std::size_t{73},
        std::size_t{4},
        true,
        "Type",
        ngl::pokepaste::Pokemon::Stats{6, 5, 4, 3, 2, 1},
        "Nature",
        ngl::pokepaste::Pokemon::Stats{1, 2, 3, 4, 5, 6},
        std::vector{
          std::string{"Attack 3"},
          std::string{"Attack 1"},
          std::string{"Attack 2"},
          std::string{"Attack 4"}
        }
      };
      CHECK_EQ(pokemon_result, pokemon_expected);
    }

    {
      const auto pokemon_value =
        "Nickname (Species) (M) @ Item\n"
        "Level: 50\n"
        "Shiny: Yes\n"
        "Happiness: 73\n"
        "Dynamax Level: 4\n"
        "Gigantamax: Yes\n"
        "Tera Type: Type\n"
        "EVs: 6 HP / 5 Atk / 4 Def / 3 SpA / 2 SpD / 1 Spe\n"
        "Nature: Nature\n"
        "IVs: 1 HP / 2 Atk / 3 Def / 4 SpA / 5 SpD / 6 Spe\n"
        "- Attack 1\n"
        "- Attack 2\n"
        "- Attack 3\n"
        "- Attack 4\n";

      try {
        (void)ngl::pokepaste::decode_pokemon(pokemon_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }

    {
      const auto pokemon_value =
        "Nickname (Species) (M) @ Item\n"
        "Ability: Ability\n"
        "Ability: Ability\n";

      try {
        (void)ngl::pokepaste::decode_pokemon(pokemon_value);
        assert(false);
      } catch ([[maybe_unused]] const std::runtime_error &e) {
      }
    }
  }
}