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
            /*struct cell_pix
            {
                std::array<uint16_t, 2> data;
            };*/

            /*struct colour
            {
                uint16_t val = 0;
            };*/
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

            //https://gist.github.com/SylvainBoilard/4645708
            constexpr std::array<uint16_t, 256> get_default_font()
            {
                /*return {
                    0xb79e, 0x388e, 0x722c, 0x75f4, 0x19bb, 0x7f8f, 0x85f9, 0xb158,
                    0x242e, 0x2400, 0x082a, 0x0800, 0x0008, 0x0000, 0x0808, 0x0808,
                    0x00ff, 0x0000, 0x00f8, 0x0808, 0x08f8, 0x0000, 0x080f, 0x0000,
                    0x000f, 0x0808, 0x00ff, 0x0808, 0x08f8, 0x0808, 0x08ff, 0x0000,
                    0x080f, 0x0808, 0x08ff, 0x0808, 0x6633, 0x99cc, 0x9933, 0x66cc,
                    0xfef8, 0xe080, 0x7f1f, 0x0701, 0x0107, 0x1f7f, 0x80e0, 0xf8fe,
                    0x5500, 0xaa00, 0x55aa, 0x55aa, 0xffaa, 0xff55, 0x0f0f, 0x0f0f,
                    0xf0f0, 0xf0f0, 0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff,
                    0x0000, 0x0000, 0x005f, 0x0000, 0x0300, 0x0300, 0x3e14, 0x3e00,
                    0x266b, 0x3200, 0x611c, 0x4300, 0x3629, 0x7650, 0x0002, 0x0100,
                    0x1c22, 0x4100, 0x4122, 0x1c00, 0x1408, 0x1400, 0x081c, 0x0800,
                    0x4020, 0x0000, 0x0808, 0x0800, 0x0040, 0x0000, 0x601c, 0x0300,
                    0x3e49, 0x3e00, 0x427f, 0x4000, 0x6259, 0x4600, 0x2249, 0x3600,
                    0x0f08, 0x7f00, 0x2745, 0x3900, 0x3e49, 0x3200, 0x6119, 0x0700,
                    0x3649, 0x3600, 0x2649, 0x3e00, 0x0024, 0x0000, 0x4024, 0x0000,
                    0x0814, 0x2241, 0x1414, 0x1400, 0x4122, 0x1408, 0x0259, 0x0600,
                    0x3e59, 0x5e00, 0x7e09, 0x7e00, 0x7f49, 0x3600, 0x3e41, 0x2200,
                    0x7f41, 0x3e00, 0x7f49, 0x4100, 0x7f09, 0x0100, 0x3e41, 0x7a00,
                    0x7f08, 0x7f00, 0x417f, 0x4100, 0x2040, 0x3f00, 0x7f08, 0x7700,
                    0x7f40, 0x4000, 0x7f06, 0x7f00, 0x7f01, 0x7e00, 0x3e41, 0x3e00,
                    0x7f09, 0x0600, 0x3e41, 0xbe00, 0x7f09, 0x7600, 0x2649, 0x3200,
                    0x017f, 0x0100, 0x3f40, 0x3f00, 0x1f60, 0x1f00, 0x7f30, 0x7f00,
                    0x7708, 0x7700, 0x0778, 0x0700, 0x7149, 0x4700, 0x007f, 0x4100,
                    0x031c, 0x6000, 0x0041, 0x7f00, 0x0201, 0x0200, 0x8080, 0x8000,
                    0x0001, 0x0200, 0x2454, 0x7800, 0x7f44, 0x3800, 0x3844, 0x2800,
                    0x3844, 0x7f00, 0x3854, 0x5800, 0x087e, 0x0900, 0x4854, 0x3c00,
                    0x7f04, 0x7800, 0x447d, 0x4000, 0x2040, 0x3d00, 0x7f10, 0x6c00,
                    0x417f, 0x4000, 0x7c18, 0x7c00, 0x7c04, 0x7800, 0x3844, 0x3800,
                    0x7c14, 0x0800, 0x0814, 0x7c00, 0x7c04, 0x0800, 0x4854, 0x2400,
                    0x043e, 0x4400, 0x3c40, 0x7c00, 0x1c60, 0x1c00, 0x7c30, 0x7c00,
                    0x6c10, 0x6c00, 0x4c50, 0x3c00, 0x6454, 0x4c00, 0x0836, 0x4100,
                    0x0077, 0x0000, 0x4136, 0x0800, 0x0201, 0x0201, 0x0205, 0x0200
                };*/

                //https://github.com/Zardoz89/dcpu_vm/blob/master/src/dcpu/devices/lem1802_font.inc
                //MIT licensed
                    return {0x000F, 0x0808, 0x080F, 0x0808, 0x08F8, 0x0808, 0x00FF, 0x0808,
                    0x0808, 0x0808, 0x08FF, 0x0808, 0x00FF, 0x1414, 0xFF00, 0xFF08,
                    0x1F10, 0x1714, 0xFC04, 0xF414, 0x1710, 0x1714, 0xF404, 0xF414,
                    0xFF00, 0xF714, 0x1414, 0x1414, 0xF700, 0xF714, 0x1417, 0x1414,
                    0x0F08, 0x0F08, 0x14F4, 0x1414, 0xF808, 0xF808, 0x0F08, 0x0F08,
                    0x001F, 0x1414, 0x00FC, 0x1414, 0xF808, 0xF808, 0xFF08, 0xFF08,
                    0x14FF, 0x1414, 0x080F, 0x0000, 0x00F8, 0x0808, 0xFFFF, 0xFFFF,
                    0xF0F0, 0xF0F0, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0x0F0F, 0x0F0F,
                    0x0000, 0x0000, 0x005f, 0x0000, 0x0300, 0x0300, 0x3e14, 0x3e00,
                    0x266b, 0x3200, 0x611c, 0x4300, 0x3629, 0x7650, 0x0002, 0x0100,
                    0x1c22, 0x4100, 0x4122, 0x1c00, 0x1408, 0x1400, 0x081c, 0x0800,
                    0x4020, 0x0000, 0x0808, 0x0800, 0x0040, 0x0000, 0x601c, 0x0300,
                    0x3e49, 0x3e00, 0x427f, 0x4000, 0x6259, 0x4600, 0x2249, 0x3600,
                    0x0f08, 0x7f00, 0x2745, 0x3900, 0x3e49, 0x3200, 0x6119, 0x0700,
                    0x3649, 0x3600, 0x2649, 0x3e00, 0x0024, 0x0000, 0x4024, 0x0000,
                    0x0814, 0x2200, 0x1414, 0x1400, 0x2214, 0x0800, 0x0259, 0x0600,
                    0x3e59, 0x5e00, 0x7e09, 0x7e00, 0x7f49, 0x3600, 0x3e41, 0x2200,
                    0x7f41, 0x3e00, 0x7f49, 0x4100, 0x7f09, 0x0100, 0x3e41, 0x7a00,
                    0x7f08, 0x7f00, 0x417f, 0x4100, 0x2040, 0x3f00, 0x7f08, 0x7700,
                    0x7f40, 0x4000, 0x7f06, 0x7f00, 0x7f01, 0x7e00, 0x3e41, 0x3e00,
                    0x7f09, 0x0600, 0x3e61, 0x7e00, 0x7f09, 0x7600, 0x2649, 0x3200,
                    0x017f, 0x0100, 0x3f40, 0x7f00, 0x1f60, 0x1f00, 0x7f30, 0x7f00,
                    0x7708, 0x7700, 0x0778, 0x0700, 0x7149, 0x4700, 0x007f, 0x4100,
                    0x031c, 0x6000, 0x417f, 0x0000, 0x0201, 0x0200, 0x8080, 0x8000,
                    0x0001, 0x0200, 0x2454, 0x7800, 0x7f44, 0x3800, 0x3844, 0x2800,
                    0x3844, 0x7f00, 0x3854, 0x5800, 0x087e, 0x0900, 0x4854, 0x3c00,
                    0x7f04, 0x7800, 0x047d, 0x0000, 0x2040, 0x3d00, 0x7f10, 0x6c00,
                    0x017f, 0x0000, 0x7c18, 0x7c00, 0x7c04, 0x7800, 0x3844, 0x3800,
                    0x7c14, 0x0800, 0x0814, 0x7c00, 0x7c04, 0x0800, 0x4854, 0x2400,
                    0x043e, 0x4400, 0x3c40, 0x7c00, 0x1c60, 0x1c00, 0x7c30, 0x7c00,
                    0x6c10, 0x6c00, 0x4c50, 0x3c00, 0x6454, 0x4c00, 0x0836, 0x4100,
                    0x0077, 0x0000, 0x4136, 0x0800, 0x0201, 0x0201, 0x0205, 0x0200};
            }

            // https://github.com/Zardoz89/dcpu_vm/blob/master/src/dcpu/devices/lem1802_palette.inc
            // MIT
            constexpr std::array<uint16_t, 16> get_default_palette()
            {
                return {0x0000,     // Black
                        0x000a,     // Blue
                        0x00a0,     // Green
                        0x00aa,     // Cyan
                        0x0a00,     // Red
                        0x0a0a,     // Magenta
                        0x0aa0,     // Yellow
                        0x0aaa,     // Light Grey
                        0x0555,     // Dark Grey
                        0x055f,     // Light Blue
                        0x05f5,     // Light Green
                        0x05ff,     // Light Cyan
                        0x0f55,     // Light Red
                        0x0f5f,     // Light Magenta
                        0x0ff5,     // Light Yellow
                        0x0fff,     // White
                        };
            }

            uint16_t palette_size = 16;
            std::array<uint16_t, 16> default_palette = get_default_palette();
            std::array<uint16_t, 256> default_cell_memory = get_default_font();

            static uint32_t to_rgba32(uint16_t in)
            {
                uint32_t r = ((in >> 8) & (0b1111)) * (256/16);
                uint32_t g = ((in >> 4) & (0b1111)) * (256/16);
                uint32_t b = ((in >> 0) & (0b1111)) * (256/16);

                uint32_t out = (r << 24) | (g << 16) | (b << 8) | 0xFF;

                return out;
            }

            //uses convention y * width + x, top left indexed
            //palette data does not use that convention unfortunately
            bool pixel_is_set(CPU& c, uint16_t font_idx, int x, int y)
            {
                if(font_map == 0)
                {
                    uint16_t lower = default_cell_memory[(font_idx % 128) * 2 + 0];
                    uint16_t upper = default_cell_memory[(font_idx % 128) * 2 + 1];

                    uint8_t o4 = lower >> 8;
                    uint8_t o3 = lower & 0xFF;
                    uint8_t o2 = upper >> 8;
                    uint8_t o1 = upper & 0xFF;

                    uint8_t octet = o4;

                    if(x == 0)
                        octet = o4;
                    if(x == 1)
                        octet = o3;
                    if(x == 2)
                        octet = o2;
                    if(x == 3)
                        octet = o1;

                    return (octet >> y) & 0x1;
                }
                else
                {
                    uint16_t addr1 = font_idx * 2 + 0 + font_map;
                    uint16_t addr2 = font_idx * 2 + 1 + font_map;

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
                    if(x == 3)
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

                        uint16_t value1 = default_cell_memory[idx * 2 + 0];
                        uint16_t value2 = default_cell_memory[idx * 2 + 1];

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

                        c.mem[addr] = default_palette[idx];
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

                for(uint16_t idx = 0; idx < (32*12); idx++)
                {
                    uint16_t addr = idx + vram_map;

                    uint16_t value = c.mem[addr];

                    uint16_t character_idx = value & 0b1111111;
                    uint16_t blink = (value >> 7) & 0x1;
                    uint16_t background_idx = (value >> 8) & 0b1111;
                    uint16_t foreground_idx = (value >> 12) & 0b1111;

                    uint16_t background_addr = background_idx + palette_map;
                    uint16_t foreground_addr = foreground_idx + palette_map;

                    uint16_t background_col = (palette_map == 0) ? default_palette[background_idx] : c.mem[background_addr];
                    uint16_t foreground_col = (palette_map == 0) ? default_palette[foreground_idx] : c.mem[foreground_addr];

                    bool would_blink = (c.cycle_count % (uint64_t)c.Hz) >= ((uint64_t)c.Hz / 2);

                    bool blink_is_blank = blink > 0 && would_blink;

                    uint32_t col_foreground32 = to_rgba32(foreground_col);
                    uint32_t col_background32 = to_rgba32(background_col);

                    if(blink_is_blank)
                    {
                        col_foreground32 = 0x000000FF;
                    }

                    for(int y=0; y < cell_height; y++)
                    {
                        for(int x=0; x < cell_width; x++)
                        {
                            int is_set = pixel_is_set(c, character_idx, x, y);

                            int big_y = (idx / cell_x_count) * cell_height + y;
                            int big_x = (idx % cell_x_count) * cell_width + x;

                            int buffer_offset = big_y * (cell_x_count * cell_width) + big_x;

                            if(is_set)
                                buffer.at(buffer_offset) = col_foreground32;
                            else
                                buffer.at(buffer_offset) = col_background32;
                        }
                    }
                }
            }

            constexpr virtual void reset() override
            {
                border_palette_index = 0;
                vram_map = 0;
                font_map = 0;
                palette_map = 0;
            }

            constexpr virtual hardware* clone() override
            {
                hardware* ret = new hardware;
                *ret = *this;

                return ret;
            }
        };
    }
}

#endif // HARDWARE_LEM1802_HPP_INCLUDED
