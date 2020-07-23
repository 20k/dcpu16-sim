#ifndef HARDWARE_LEM1802_HPP_INCLUDED
#define HARDWARE_LEM1802_HPP_INCLUDED

#include "base_hardware.hpp"
#include "base_sim.hpp"
#include <array>

namespace dcpu
{
    namespace sim
    {
        struct detail
        {
            struct cell_pix
            {
                std::array<uint16_t, 2> data;
            };

            struct colour
            {
                uint16_t val = 0;
            };
        };

        struct LEM1802 : hardware
        {
            int cell_x_count = 32;
            int cell_y_count = 12;

            int cell_width = 4;
            int cell_height = 8;

            uint16_t border_palette_index = 0;

            uint16_t vram_map = 0;
            uint16_t font_map = 0;
            uint16_t palette_map = 0;

            uint16_t palette_size = 16;
            std::array<detail::colour, 16> default_palette;

            std::array<detail::cell_pix, 128> default_cell_memory;

            //uses convention y * width + x, top left indexed
            //palette data does not use that convention unfortunately
            bool pixel_is_set(CPU& c, uint16_t palette_idx, int x, int y)
            {
                if(palette_map == 0)
                {
                    detail::cell_pix& val = default_cell_memory[palette_idx % 128];

                    uint8_t o4 = val.data[0] >> 8;
                    uint8_t o3 = val.data[0] & 0xFF;
                    uint8_t o2 = val.data[1] >> 8;
                    uint8_t o1 = val.data[1] & 0xFF;

                    uint8_t octet = o4;

                    if(x == 0)
                        octet = o4;
                    if(x == 1)
                        octet = o3;
                    if(x == 2)
                        octet = o2;
                    if(x == 1)
                        octet = o1;

                    return (octet >> y) & 0x1;
                }
                else
                {
                    uint16_t addr1 = palette_idx * 2 + 0 + palette_map;
                    uint16_t addr2 = palette_idx * 2 + 1 + palette_map;

                    uint8_t o4 = c.mem[addr1] >> 8;
                    uint8_t o3 = c.mem[addr1] & 0xFF;
                    uint8_t o2 = c.mem[addr2] >> 8;
                    uint8_t o1 = c.mem[addr2] & 0xFF;

                    uint8_t octet = o4;

                    if(x == 0)
                        octet = o4;
                    if(x == 1)
                        octet = o3;
                    if(x == 2)
                        octet = o2;
                    if(x == 1)
                        octet = o1;

                    return (octet >> y) & 0x1;
                }
            }

            LEM1802()
            {
                hardware_id = 0x7349f615;
                hardware_version = 0x1802;
                manufacturer_id = 0x1c6c8b36;
            }

            constexpr virtual void interrupt(world_base* state, CPU& c)
            {
                if(c.regs[A_REG] == 0)
                {
                    vram_map = c.regs[B_REG];
                }

                if(c.regs[A_REG] == 1)
                {
                    font_map = c.regs[B_REG];
                }

                if(c.regs[A_REG] == 2)
                {
                    palette_map = c.regs[B_REG];
                }

                if(c.regs[A_REG] == 3)
                {
                    border_palette_index = c.regs[B_REG] & 0xF;
                }

                // Dump font
                if(c.regs[A_REG] == 4)
                {
                    uint16_t memory_address = c.regs[B_REG];

                    for(uint16_t idx = 0; idx < default_cell_memory.size(); idx++)
                    {
                        uint16_t addr1 = idx * 2 + 0 + memory_address;
                        uint16_t addr2 = idx * 2 + 1 + memory_address;

                        uint16_t value1 = default_cell_memory[idx].data[0];
                        uint16_t value2 = default_cell_memory[idx].data[1];

                        c.mem[addr1] = value1;
                        c.mem[addr2] = value2;
                    }

                    c.next_instruction_cycle += 256;
                }

                // Dump palette
                if(c.regs[A_REG] == 5)
                {
                    uint16_t memory_address = c.regs[B_REG];

                    for(uint16_t idx = 0; idx < default_palette.size(); idx++)
                    {
                        uint16_t addr = idx + memory_address;

                        c.mem[addr] = default_palette[idx].val;
                    }

                    c.next_instruction_cycle += 16;
                }
            }

            constexpr virtual void step(world_base* state, CPU& c) override
            {

            }

            constexpr void render(world_base* state, CPU& c, std::array<uint32_t, 128*96>& buffer)
            {
                if(vram_map == 0)
                    return;

                for(uint16_t idx = 0; idx < default_cell_memory.size(); idx++)
                {
                    uint16_t addr = idx + vram_map;

                    uint16_t character_idx = addr & 0b1111111;
                    uint16_t blink = (addr >> 7) & 0x1;
                    uint16_t background_idx = (addr >> 8) & 0b1111;
                    uint16_t foreground_idx = (addr >> 12) & 0b1111;

                    uint16_t background_addr = background_idx + palette_map;
                    uint16_t foreground_addr = foreground_idx + palette_map;

                    uint16_t background_col = (palette_map == 0) ? default_palette[background_idx].val : c.mem[background_addr];
                    uint16_t foreground_col = (palette_map == 0) ? default_palette[foreground_idx].val : c.mem[foreground_addr];


                }
            }
        };
    }
}

#endif // HARDWARE_LEM1802_HPP_INCLUDED
