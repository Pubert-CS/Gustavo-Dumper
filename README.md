# Gustavo Dumper
(Named Gustavo Dumper for lack of a better name)

What Gustavo Dumper is an attempt for an il2cpp dumper that dumps the following files:
- dump.cs
- script.json
- il2cpp.h

that should be supported by the Il2CppDumper ida_with_struct files from [Il2CppDumper](https://github.com/Perfare/Il2CppDumper/)

Currently, Gustavo Dumper isn't made for easy to be changed il2cpp version. Currently, the best way to change the version is to edit il2cpp-headers.h to be your game's unity version. This file can be retrieved by [Il2CppVersions](https://github.com/nneonneo/Il2CppVersions)
The Il2Cpp header in StructGen.hpp is currently configured for metadata version 31, but can be changed by referencing other dumpers, or adding it yourself.

## TODO
- Make the script.json output based off of [Redux](https://github.com/LukeFZ/Il2CppInspectorRedux) instead of [Dumper](https://github.com/Perfare/Il2CppDumper/)
- Support runtime string output