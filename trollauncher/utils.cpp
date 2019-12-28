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
#include <vector>

namespace tl {

namespace {

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

}  // namespace

std::optional<std::string> GetEnvironmentVar(const std::string& name)
{
  const char* var_ptr = std::getenv(name.data());
  if (var_ptr == nullptr) {
    return std::nullopt;
  }
  return var_ptr;
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

}  // namespace tl
