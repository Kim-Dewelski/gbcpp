#include<memory/memory.h>
#include<fstream>
#include<iostream>

uint8_t memory_t::read(uint16_t adr){
    __DEBUG_LINE(
        if(read_breakpoints.size() > 0){
            if(read_breakpoints.contains(adr)){
                if(read_breakpoints[adr] && read_breakpoint_callbk)
                    read_breakpoint_callbk(adr);
            }
        }
    )
    switch(adr){
    case 0x100:             if(boot_rom_bound) unbind_boot_rom();
    case 0x0000 ... 0x00FF:
    case 0x0101 ... 0x3FFF:
                            return rom1[adr];
    case 0x4000 ... 0x7FFF: return rom2.get()[adr-0x4000];
    case 0x8000 ... 0x9FFF: return vram[adr-0x8000];
    case 0xA000 ... 0xBFFF: return ram_banks.get()[adr-0xA000];
    case 0xC000 ... 0xCFFF: return wram[adr-0xC000];
    case 0xD000 ... 0xDFFF: return wram_banks.get()[adr-0xD000];
    //case 0xE000 ... 0xFDFF: 
    case 0xFE00 ... 0xFE9F: return oam[adr-0xFE00];
    case 0xFF00 ... 0xFF7F: return io_regs[adr-0xFF00];
    case 0xFF80 ... 0xFFFE: return hram[adr-0xFF80];
    }
    return 0xFF;
}

uint8_t memory_t::debug_read(uint16_t adr){ //  doesn't unbind boot rom.
    switch(adr){
    case 0x0000 ... 0x3FFF: return rom1[adr];
    case 0x4000 ... 0x7FFF: return rom2.get()[adr-0x4000];
    case 0x8000 ... 0x9FFF: return vram[adr-0x8000];
    case 0xA000 ... 0xBFFF: return ram_banks.get()[adr-0xA000];
    case 0xC000 ... 0xCFFF: return wram[adr-0xC000];
    case 0xD000 ... 0xDFFF: return wram_banks.get()[adr-0xD000];
    //case 0xE000 ... 0xFDFF: 
    case 0xFE00 ... 0xFE9F: return oam[adr-0xFE00];
    case 0xFF00 ... 0xFF7F: return io_regs[adr-0xFF00];
    case 0xFF80 ... 0xFFFE: return hram[adr-0xFF80];
    }
    return 0xFF;
}

void memory_t::write(uint16_t adr, uint8_t val){
    __DEBUG_LINE(
        if(adr == 0xFF01)
            std::cout << val;   //  tmp
        if(write_breakpoints.size() > 0){
            if(write_breakpoints.contains(adr)){
                if(write_breakpoints[adr] && write_breakpoint_callbk)
                    write_breakpoint_callbk(adr, val);
            }
        }
    )
    switch(adr){
    case 0x0000 ... 0x7FFF: on_rom_write(adr);                     break;
    case 0x8000 ... 0x9FFF: vram[adr-0x8000]                = val; break;
    case 0xA000 ... 0xBFFF: ram_banks.get()[adr-0xA000]     = val; break;
    case 0xC000 ... 0xCFFF: wram[adr-0xC000]                = val; break;
    case 0xD000 ... 0xDFFF: wram_banks.get()[adr-0xD000]    = val; break;
    //case 0xE000 ... 0xFDFF:
    case 0xFE00 ... 0xFE9F: oam[adr-0xFE00]                 = val; break;
    case 0xFF00 ... 0xFF7F: io_regs[adr-0xFF00]             = val; break;
    case 0xFF80 ... 0xFFFE: hram[adr-0xFF80]                = val; break;
    }
}

void memory_t::load_rom(std::string path){
    std::ifstream stream{path, std::ios::binary};
    if(!stream)
        throw std::runtime_error("unable to locate rom");
    stream.read(reinterpret_cast<char*>(rom1.data()), rom1.size());
    parse_rom_info();
    std::vector<rom_bank_t> banks_vec;
    banks_vec.reserve((2<<info.rom_size)-1);
    for(rom_bank_t bank; !stream.eof(); banks_vec.push_back(bank),bank={})
        stream.read(reinterpret_cast<char*>(bank.data()), bank.size());
    if(banks_vec.size())
        banks_vec.pop_back();
    rom2.copy_from_vec(banks_vec);
    //  binds the boot rom to rom but saves the unbinded rom to another array.
    bind_boot_rom();
}

void memory_t::bind_boot_rom(){
    boot_rom_bound = true;
    std::copy(rom1.begin(), rom1.end(), unbinded_rom.begin());
    std::ifstream boot_rom_stream{"roms/dmg_boot.bin", std::ios::binary};
    if(!boot_rom_stream)
        throw std::runtime_error("unable to locate boot rom file!");
    boot_rom_stream.read(reinterpret_cast<char*>(rom1.data()),256);
}

void memory_t::unbind_boot_rom(){
    boot_rom_bound = false;
    std::copy(unbinded_rom.begin(), unbinded_rom.end(), rom1.begin());
    if(unbind_bootrom_callbk)
        unbind_bootrom_callbk();
}

void memory_t::on_rom_write(uint16_t adr){

}

void memory_t::parse_rom_info(){
    std::copy(&rom1[0x0134], &rom1[0x014F], reinterpret_cast<uint8_t*>(&info));
}