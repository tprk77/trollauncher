// Copyright (c) 2019 Tim Perkins

// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the “Software”), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
// to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "trollauncher/utils.hpp"

#include <cstdlib>

#include <boost/filesystem.hpp>

namespace tl {

namespace {

namespace fs = std::filesystem;
namespace bfs = boost::filesystem;

static const std::vector<char> alpha_numerics = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
};

static const std::vector<std::string> big_list_o_rocks = {
    "A'a",
    "Adakite",
    "Alkali Feldspar Granite",
    "Amphibolite",
    "Andesite",
    "Anorthosite",
    "Anthracite",
    "Aplite",
    "Argillite",
    "Arkose",
    "Banded Iron Formation",
    "Basalt",
    "Basaltic Trachyandesite",
    "Basanite",
    "Benmoreite",
    "Blairmorite",
    "Blueschist",
    "Boninite",
    "Breccia",
    "Calcarenite",
    "Calcflinta",
    "Carbonatite",
    "Cataclasite",
    "Chalk",
    "Charnockite",
    "Chert",
    "Claystone",
    "Coal",
    "Comendite",
    "Conglomerate",
    "Coquina",
    "Dacite",
    "Diabase",
    "Diamictite",
    "Diatomite",
    "Diorite",
    "Dolomite",
    "Dunite",
    "Eclogite",
    "Enderbite",
    "Essexite",
    "Evaporite",
    "Flint",
    "Foidolite",
    "Gabbro",
    "Geyserite",
    "Gneiss",
    "Granite",
    "Granodiorite",
    "Granophyre",
    "Granulite",
    "Greenschist",
    "Greywacke",
    "Gritstone",
    "Harzburgite",
    "Hawaiite",
    "Hornblendite",
    "Hornfels",
    "Hyaloclastite",
    "Icelandite",
    "Ignimbrite",
    "Ijolite",
    "Itacolumite",
    "Jaspillite",
    "Kimberlite",
    "Komatiite",
    "Lamproite",
    "Lamprophyre",
    "Laterite",
    "Latite",
    "Lherzolite",
    "Lignite",
    "Limestone",
    "Litchfieldite",
    "Marble",
    "Marl",
    "Metapelite",
    "Metapsammite",
    "Migmatite",
    "Monzogranite",
    "Monzonite",
    "Mudstone",
    "Mugearite",
    "Mylonite",
    "Napoleonite",
    "Nepheline Syenite",
    "Nephelinite",
    "Norite",
    "Obsidian",
    "Oil Shale",
    "Oolite",
    "Pahoehoe",
    "Pantellerite",
    "Pegmatite",
    "Peridotite",
    "Phonolite",
    "Phonotephrite",
    "Phosphorite",
    "Phyllite",
    "Picrite",
    "Porphyry",
    "Pseudotachylite",
    "Pumice",
    "Pyroxenite",
    "Quartz Diorite",
    "Quartz Monzonite",
    "Quartzite",
    "Quartzolite",
    "Rhyodacite",
    "Rhyolite",
    "Sandstone",
    "Schist",
    "Scoria",
    "Serpentinite",
    "Shale",
    "Shonkinite",
    "Shoshonite",
    "Siltstone",
    "Skarn",
    "Slate",
    "Soapstone",
    "Sovite",
    "Suevite",
    "Syenite",
    "Sylvinite",
    "Tachylyte",
    "Talc Carbonate",
    "Tectonite",
    "Tephriphonolite",
    "Tephrite",
    "Tillite",
    "Tonalite",
    "Trachyandesite",
    "Trachybasalt",
    "Trachyte",
    "Travertine",
    "Troctolite",
    "Trondhjemite",
    "Tufa",
    "Tuff",
    "Turbidite",
    "Wackestone",
    "Websterite",
    "Wehrlite",
    "Whiteschist",
};

static const std::vector<std::string> default_launcher_icons = {
    // Line 1
    "Bedrock",
    "Bookshelf",
    "Brick",
    "Cake",
    "Carved_Pumpkin",
    "Chest",
    "Clay",
    "Coal_Block",
    "Coal_Ore",
    "Cobblestone",
    // Line 2
    "Crafting_Table",
    "Creeper_Head",
    "Diamond_Block",
    "Diamond_Ore",
    "Dirt",
    "Dirt_Podzol",
    "Dirt_Snow",
    "Emerald_Block",
    "Emerald_Ore",
    "Enchanting_Table",
    "End_Stone",
    // Line 3
    "Farmland",
    "Furnace",
    "Furnace_On",
    "Glass",
    "Glazed_Terracotta_Light_Blue",
    "Glazed_Terracotta_Orange",
    "Glazed_Terracotta_White",
    "Glowstone",
    "Gold_Block",
    "Gold_Ore",
    "Grass",
    // Line 4
    "Gravel",
    "Hardened_Clay",
    "Ice_Packed",
    "Iron_Block",
    "Iron_Ore",
    "Lapis_Ore",
    "Leaves_Birch",
    "Leaves_Jungle",
    "Leaves_Oak",
    "Leaves_Spruce",
    "Lectern_Book",
    // Line 5
    "Log_Acacia",
    "Log_Birch",
    "Log_DarkOak",
    "Log_Jungle",
    "Log_Oak",
    "Log_Spruce",
    "Mycelium",
    "Nether_Brick",
    "Netherrack",
    "Obsidian",
    "Planks_Acacia",
    // Line 6
    "Planks_Birch",
    "Planks_DarkOak",
    "Planks_Jungle",
    "Planks_Oak",
    "Planks_Spruce",
    "Quartz_Ore",
    "Red_Sand",
    "Red_Sandstone",
    "Redstone_Block",
    "Redstone_Ore",
    "Sand",
    // Line 7
    "Sandstone",
    "Skeleton_Skull",
    "Snow",
    "Soul_Sand",
    "Stone",
    "Stone_Andesite",
    "Stone_Diorite",
    "Stone_Granite",
    "TNT",
    "Water",
    "Wool",
};

}  // namespace

std::optional<std::string> GetEnvironmentVar(const std::string& name)
{
  const char* var_ptr = std::getenv(name.data());
  if (var_ptr == nullptr) {
    return std::nullopt;
  }
  return var_ptr;
}

std::optional<fs::path> CreateTempDir()
{
  // Ok, so this sucks because we can't use "mkdtemp" due to that not existing on MinGW, etc. And
  // using "tmpnam" also sucks because it's obsolete. So I guess we will just use Boost.
  std::error_code fs_ec;
  const fs::path temp_path = fs::temp_directory_path(fs_ec);
  if (fs_ec) {
    return std::nullopt;
  }
  boost::system::error_code bfs_ec;
  const bfs::path unique_bpath = bfs::unique_path("TL-%%%%-%%%%-%%%%-%%%%", bfs_ec);
  if (bfs_ec) {
    return std::nullopt;
  }
  const fs::path unique_path = unique_bpath.string();
  const fs::path dest_path = temp_path / unique_path;
  fs::create_directories(dest_path, fs_ec);
  if (fs_ec) {
    return std::nullopt;
  }
  fs::permissions(dest_path, fs::perms::owner_all, fs_ec);
  if (fs_ec) {
    return std::nullopt;
  }
  return dest_path;
}

std::string GetRandomId()
{
  std::string id;
  for (int ii = 0; ii < 32; ++ii) {
    const int rand_an_index = std::rand() % alpha_numerics.size();
    id += alpha_numerics.at(rand_an_index);
  }
  return id;
}

std::string GetRandomName()
{
  const int rand_rock_index = std::rand() % big_list_o_rocks.size();
  const int rand_number = std::rand() % 100;
  return big_list_o_rocks.at(rand_rock_index) + " " + std::to_string(rand_number);
}

std::vector<std::string> GetDefaultLauncherIcons()
{
  return default_launcher_icons;
}

}  // namespace tl
