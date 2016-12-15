#include "flatbuffers/flatbuffers.h"
#include "test_generated.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace Test;

int main(int arg, char ** argv) {
    if (arg == 1) {
        flatbuffers::FlatBufferBuilder builder(1024);

        auto weapon_one = builder.CreateString("Nope");
        auto weapon_two = builder.CreateString("Nothing");
        auto nope = CreateWeapon(builder, weapon_one, 3);
        auto nothing = CreateWeapon(builder, weapon_two, 5);

        auto name = builder.CreateString("Orc");
        unsigned char treasure[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto inventory = builder.CreateVector(treasure, 10);

        std::vector<flatbuffers::Offset<Weapon>> weapons_vector;
        weapons_vector.push_back(nope);
        weapons_vector.push_back(nothing);
        auto weapons = builder.CreateVector(weapons_vector);

        int hp = 300;
        int mana = 150;

        auto vec = Vec3(1.0f, 2.0f, 3.0f);

        auto orc = CreateMonster(builder, &vec, mana, hp, name,
                                inventory, Color_Red, weapons, Equipment_Weapon,
                                nothing.Union());
        builder.Finish(orc);
        char * buf = (char*)builder.GetBufferPointer();
        int size = builder.GetSize();

        {
            std::ofstream f( "my_monster.bin", std::ios_base::out | std::ios_base::binary );
            f.write(buf, size);
        }

        std::cout << "Wrote " << size << " bytes to my_monster.bin!" << std::endl;
    } else {
        std::unique_ptr<char[]> buffer;
        size_t size = 0;
        {
            std::ifstream f( "my_monster.bin", std::ios_base::ate | std::ios_base::binary );
            if (f) {
                size = f.tellg();
                buffer = std::unique_ptr<char[]>(new char[size]);
                f.seekg(0); //rewind.
                f.read(buffer.get(), size);
            } else {
                throw std::runtime_error("Cannot open file.");
            }
        }

        auto monster = GetMonster(buffer.get());

        std::cout << "My monster is named: " << monster->name()->c_str() << std::endl;
        auto union_type = monster->equipped_type();

        if (union_type == Equipment_Weapon) {
            auto weapon = static_cast<const Weapon*>(monster->equipped());

            auto weapon_name = weapon->name()->c_str();
            std::cout << "He is carrying a " << weapon_name << std::endl;
        }
    }
    return 0;
}