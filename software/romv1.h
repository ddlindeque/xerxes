template<typename TRom> void initialize_kernel_rom(TRom *rom)
{
    rom->program(0xE000, 0xA2);
    rom->program(0xE001, 0xFF);
    rom->program(0xE002, 0x9A);
    rom->program(0xE003, 0x20);
    rom->program(0xE004, 0x00);
    rom->program(0xE005, 0xE1);
    rom->program(0xE006, 0x58);
    rom->program(0xE007, 0xEA);
    rom->program(0xE008, 0x4C);
    rom->program(0xE009, 0x07);
    rom->program(0xE00A, 0xE0);

    rom->program(0xE100, 0xA9);
    rom->program(0xE101, 0x20);
    rom->program(0xE102, 0x85);
    rom->program(0xE103, 0x04);
    rom->program(0xE104, 0xA9);
    rom->program(0xE105, 0xE8);
    rom->program(0xE106, 0x85);
    rom->program(0xE107, 0x05);
    rom->program(0xE108, 0xA9);
    rom->program(0xE109, 0x21);
    rom->program(0xE10A, 0x85);
    rom->program(0xE10B, 0x06);
    rom->program(0xE10C, 0xA9);
    rom->program(0xE10D, 0xE8);
    rom->program(0xE10E, 0x85);
    rom->program(0xE10F, 0x07);
    rom->program(0xE110, 0xA9);
    rom->program(0xE111, 0x30);
    rom->program(0xE112, 0x85);
    rom->program(0xE113, 0x08);
    rom->program(0xE114, 0xA9);
    rom->program(0xE115, 0xE8);
    rom->program(0xE116, 0x85);
    rom->program(0xE117, 0x09);
    rom->program(0xE118, 0xA9);
    rom->program(0xE119, 0x40);
    rom->program(0xE11A, 0x85);
    rom->program(0xE11B, 0x0A);
    rom->program(0xE11C, 0xA9);
    rom->program(0xE11D, 0xE8);
    rom->program(0xE11E, 0x85);
    rom->program(0xE11F, 0x0B);
    rom->program(0xE120, 0xA9);
    rom->program(0xE121, 0x50);
    rom->program(0xE122, 0x85);
    rom->program(0xE123, 0x0C);
    rom->program(0xE124, 0xA9);
    rom->program(0xE125, 0xE8);
    rom->program(0xE126, 0x85);
    rom->program(0xE127, 0x0D);
    rom->program(0xE128, 0xA9);
    rom->program(0xE129, 0x01);
    rom->program(0xE12A, 0x8D);
    rom->program(0xE12B, 0x2F);
    rom->program(0xE12C, 0xD0);
    rom->program(0xE12D, 0x20);
    rom->program(0xE12E, 0x40);
    rom->program(0xE12F, 0xE1);
    rom->program(0xE130, 0x60);

    rom->program(0xE140, 0xA9);
    rom->program(0xE141, 0x02);
    rom->program(0xE142, 0x8D);
    rom->program(0xE143, 0x2F);
    rom->program(0xE144, 0xD0);
    rom->program(0xE145, 0x60);

    rom->program(0xE800, 0x20);
    rom->program(0xE801, 0x10);
    rom->program(0xE802, 0xE8);
    rom->program(0xE803, 0x58);
    rom->program(0xE804, 0x40);

    rom->program(0xE810, 0xAD);
    rom->program(0xE811, 0x30);
    rom->program(0xE812, 0xD0);
    rom->program(0xE813, 0x0A);
    rom->program(0xE814, 0xAA);
    rom->program(0xE815, 0x7C);
    rom->program(0xE816, 0x04);

    rom->program(0xE820, 0x60);

    rom->program(0xE821, 0xAD);
    rom->program(0xE822, 0x31);
    rom->program(0xE823, 0xD0);
    rom->program(0xE824, 0xA4);
    rom->program(0xE825, 0x03);
    rom->program(0xE826, 0x91);
    rom->program(0xE827, 0x0E);
    rom->program(0xE828, 0xE6);
    rom->program(0xE829, 0x03);
    rom->program(0xE82A, 0x20);
    rom->program(0xE82B, 0x40);
    rom->program(0xE82C, 0xE1);
    rom->program(0xE82D, 0x60);

    rom->program(0xE830, 0xAD);
    rom->program(0xE831, 0x31);
    rom->program(0xE832, 0xD0);
    rom->program(0xE833, 0x85);
    rom->program(0xE834, 0x0E);
    rom->program(0xE835, 0x20);
    rom->program(0xE836, 0x40);
    rom->program(0xE837, 0xE1);
    rom->program(0xE838, 0x60);

    rom->program(0xE840, 0xAD);
    rom->program(0xE841, 0x31);
    rom->program(0xE842, 0xD0);
    rom->program(0xE843, 0x85);
    rom->program(0xE844, 0x0F);
    rom->program(0xE845, 0xA9);
    rom->program(0xE846, 0x00);
    rom->program(0xE847, 0x85);
    rom->program(0xE848, 0x03);
    rom->program(0xE849, 0x20);
    rom->program(0xE84A, 0x40);
    rom->program(0xE84B, 0xE1);
    rom->program(0xE84C, 0x60);

    rom->program(0xE850, 0x58);
    rom->program(0xE851, 0x6C);
    rom->program(0xE852, 0x0E);

    rom->program(0xFFFA, 0x00);
    rom->program(0xFFFB, 0x00);
    rom->program(0xFFFC, 0x00);
    rom->program(0xFFFD, 0xE0);
    rom->program(0xFFFE, 0x00);
    rom->program(0xFFFF, 0xE8);
}