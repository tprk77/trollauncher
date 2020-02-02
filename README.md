<p align="center">
    <a href="https://github.com/tprk77/trollauncher">
        <img src="resource/banner_big.png" alt="Trollauncher Banner" />
    </a>
</p>
<p align="center">
    <a href="https://github.com/tprk77/trollauncher/releases">
        <b>&gt;&gt; Download &lt;&lt;</b>
    </a>
</p>

<br/>

## Overview ##

Trollauncher is a modpack installer for the "Vanilla" Minecraft Launcher.

It's a simple, minimalist utility to make installing modpacks quick and easy.

<p align="center">
    <img src="resource/screenshot.png" alt="Screenshot" />
</p>

It will create a new profile, extract the modpack files, and if necessary,
install the right version of Forge.

**NOTE:** Trollauncher only works for modpack zip files that conform to a
specific layout! It does not work for arbitrary modpacks. Please read [Making
Modpacks](#making-modpacks) for more information.

## Demo ##

<p align="center">
    <a href="https://youtu.be/L-zkzFXI994">
        <img src="resource/trollauncher_demo.gif" alt="Trollauncher Demo" />
    </a>
</p>

## Supported Software ##

| Java Edition | Bedrock Edition |
| ------------ | --------------- |
| YES          | NO              |

| Windows                 | Linux       | MacOs |
| ----------------------- |------------ |------ |
| YES (Stand-alone `exe`) | YES (`deb`) | NO    |

| Forge               | Fabric |
| ------------------- | ------ |
| YES (`>= MC 1.5.2`) | NO     |

## Features ##

* Works with Forge for all recent versions of Minecraft.
* Works together with the "Vanilla" Minecraft Launcher.
* Easy to use, better than manually installing mods.
* Easy to create modpacks, [just zip 'em up](#making-modpacks).

The "Vanilla" Minecraft Launcher is perfectly fine, but requires modpacks to be
installed manually. Manually installing modpacks is tedious, and mistakes can
introduce subtle errors that break the game. Other launchers like MultiMC don't
work with Forge beyond Minecraft 1.12 (at the time of writing).

### Versus Twitch ###

The Twitch App works with the latest Forge, so why not use that?

If you're happy with Twitch, then use Twitch. But it has two potential pitfalls:

* You have to accept [Twitch's Privacy Policy][twitch_privacy_notice].
* It doesn't work on Linux.

## Know Issues & Planned Features ##

* The Forge installer isn't automated. You still have to click through it.
* You can't update an existing profile (yet).
* Modpacks can't use `universal.jar` with older Forge (yet).
* Trollauncher isn't ported to MacOs (yet).

## Why the name? ##

Because I'm a fan of classic memes.

> It's an older meme, sir, but it checks out.

## Making Modpacks ##

Making a modpack is easy. You just need to zip up some files.

A typical modpack will look something like this:

```
My_Modpack_Xyz_123/
├── config
│   └── ...
├── mods
│   └── ...
└── trollauncher
    └── installer.jar
```

**NOTE:** `config`, `mods`, and `trollauncher` directory names are
case-sensitive.

The top-level directory is optional, and can be named whatever you want (it's
not used for anything). The `mods` directory is where you should place all the
`jar` files for each of your mods. The `config` directory is where mods
typically locate their config files. There can also be additional directories
and files as needed. They will just get extracted with everything else.

The only slightly tricky thing is `trollauncher/installer.jar`, which must be a
valid Forge installer. At the moment, only Forge is supported. Trollauncher will
look for this file to determine the target Forge version of the modpack. It will
automatically start the Forge installer if it needs to be installed.

To give an example, you should download a Forge installer, e.g.,
`forge-1.14.4-28.1.0-installer.jar` and rename it to
`trollauncher/installer.jar` before zipping everything up. It must be a
`*-installer.jar`, it CANNOT be a `*-universal.jar`, even for older versions of
Forge where that would work. (This is a limitation of the current
implementation.)

**NOTE:** Please make sure that all your included mods allow for redistribution.
This is true for open source mods using popular licenses such as MIT, BSD,
Apache, etc. However, it is not true for some proprietary mods, or some mods
using wonky one-off licenses. Please beware!

## Developer Info ##

If you're a programmer, you can build Trollauncher yourself.

(Otherwise just download it from the [release page!][trollauncher_releases])

### Building On Ubuntu ###

* `apt-get install build-essential`
* `apt-get install python3-pip`
* `pip3 install meson`
* `apt-get install ninja-build`
* `apt-get install libboost-all-dev`
* `apt-get install libzip-dev`
* `apt-get install libwxgtk3.0-dev`
* `cd trollauncher`
* `meson build`
* `ninja -C build`

### Building On Windows ###

* Install MSYS2
* Start the MSYS2 MINGW64 Shell
* `pacman -Syu`
* `pacman -Syu` (again)
* `pacman -S mingw64/mingw-w64-x86_64-toolchain`
* `pacman -S mingw64/mingw-w64-x86_64-python-pip`
* `pip3 install meson`
* `pacman -S mingw64/mingw-w64-x86_64-ninja`
* `pacman -S mingw64/mingw-w64-x86_64-boost`
* `pacman -S mingw64/mingw-w64-x86_64-libzip`
* `pacman -S mingw64/mingw-w64-x86_64-wxWidgets`
* `cd trollauncher`
* `meson build`
* `ninja -C build`

## License ##

Trollauncher uses an MIT license. See `LICENSE.md` for details.

<!-- Links -->

[trollauncher_releases]: https://github.com/tprk77/trollauncher/releases
[twitch_privacy_notice]: https://www.twitch.tv/p/legal/privacy-notice

<!-- Local Variables: -->
<!-- fill-column: 80 -->
<!-- End: -->
