libma
=====

Decompilation of libma, the Mobile Adapter client library shipped with AGB SDK 3.1 (JP).

This library was used by various GBA games to implement support for the [Mobile Adapter GB](https://bulbapedia.bulbagarden.net/wiki/Mobile_Game_Boy_Adapter), allowing games to reach the internet through japanese phones, and implement cross-country multiplayer, rankings and events.

A full list of the supported games can be found [here](https://bulbapedia.bulbagarden.net/wiki/Mobile_Game_Boy_Adapter#List_of_compatible_software).

How to build
------------

* Build the [010110-ThumbPatch toolchain](https://github.com/mid-kid/arm-000512/tree/010110-ThumbPatch) using `./configure --prefix=<install dir> --host=i686-linux-gnu --target=thumb-elf`, and add it to `$PATH`.
* Run `make`.
