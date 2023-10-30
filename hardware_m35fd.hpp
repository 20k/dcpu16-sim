#ifndef HARDWARE_M35FD_HPP_INCLUDED
#define HARDWARE_M35FD_HPP_INCLUDED

namespace dcpu
{
    namespace sim
    {
        struct floppy
        {
            static constexpr int tracks = 80;
            static constexpr int sectors_per_track = 18;
            static constexpr int words_per_sector = 512;
            static constexpr int words = tracks * sectors_per_track * words_per_sector;

            int last_track = 0;
            //std::array<std::array<std::array<uint16_t, 512>, 18>, 80> data;
            std::array<std::array<uint16_t, 512>, tracks * sectors_per_track> data;

            /*void set_word(int word_offset, uint16_t word)
            {
                int lword = word_offset % words_per_sector;
                int sector = (word_offset / words_per_sector) % sectors_per_track;
                int track = (word_offset / words_per_sector) / sectors_per_track;

                data.at(track).at(sector).at(lword) = word;
            }*/

            void set_word(uint16_t word_offset, uint16_t sector, uint16_t word)
            {
                word_offset = word_offset % words_per_sector;
                sector = sector % data.size();

                data.at(sector).at(word_offset) = word;
            }

            /*uint16_t get_word(int word_offset)
            {
                int lword = word_offset % words_per_sector;
                int sector = (word_offset / words_per_sector) % sectors_per_track;
                int track = (word_offset / words_per_sector) / sectors_per_track;

                return data.at(track).at(sector).at(lword);
            }*/

            uint16_t get_word(uint16_t word_offset, uint16_t sector)
            {
                word_offset = word_offset % words_per_sector;
                sector = sector % data.size();

                return data.at(sector).at(word_offset);
            }

            ///data, should_seek
            /*std::optional<std::array<uint16_t, 512>> get_sector(int sector)
            {
                int track = sector / sectors_per_track;
                int sector_offset = sector % sectors_per_track;

                if(track >= 80 || sector_offset >= 16)
                    return std::nullopt;

                return data.at(track).at(sector_offset);
            }*/

            /*bool needs_seek(int sector)
            {
                int track = sector / sectors_per_track;

                return track != last_track;
            }

            void seek(int sector)
            {
                int track = sector / sectors_per_track;

                last_track = track;
            }*/
        };

        namespace M35FD_common
        {
            static constexpr uint16_t STATE_NO_MEDIA = 0;
            static constexpr uint16_t STATE_READY = 1;
            static constexpr uint16_t STATE_READY_WP = 2;
            static constexpr uint16_t STATE_BUSY = 3;

            static constexpr uint16_t ERROR_NONE = 0;
            static constexpr uint16_t ERROR_BUSY = 1;
            static constexpr uint16_t ERROR_NO_MEDIA = 2;
            static constexpr uint16_t ERROR_PROTECTED = 3;
            static constexpr uint16_t ERROR_EJECT = 4;
            static constexpr uint16_t ERROR_BAD_SECTOR = 5;
            static constexpr uint16_t ERROR_BROKEN = 0xffff;
        }

        struct M35FD_simple : hardware
        {
            M35FD_simple()
            {
                hardware_id = 0x4fd524c5;
                hardware_version = 0x000a;
                manufacturer_id = 0x1eb37e91;
            }

            uint16_t current_state = M35FD_common::STATE_READY;
            uint16_t last_error = M35FD_common::ERROR_NONE;

            std::optional<floppy> floppy_opt;

            bool interrupts_enabled = false;
            uint16_t interrupt_msg = 0;

            void trigger_interrupt(CPU& c)
            {
                if(!interrupts_enabled)
                    return;

                interrupt_type type;
                type.is_software = 0;
                type.message = interrupt_msg;

                c.add_interrupt(type);
            }

            ///doesn't trigger an interrupt because it never happens
            void insert_floppy(const floppy& flop)
            {
                floppy_opt = flop;
                current_state = M35FD_common::STATE_READY;
            }

            virtual void interrupt2(std::span<hardware*> all_hardware, world_base* state, CPU& c)
            {
                using namespace M35FD_common;

                if(c.regs[A_REG] == 0)
                {
                    c.regs[B_REG] = current_state;
                    c.regs[C_REG] = last_error;
                    last_error = ERROR_NONE;
                }

                if(c.regs[A_REG] == 1)
                {
                    interrupts_enabled = c.regs[X_REG] != 0;
                    interrupt_msg = c.regs[X_REG];
                }

                if(c.regs[A_REG] == 2)
                {
                    c.regs[B_REG] = 1;
                    uint16_t reading_sector = c.regs[X_REG];
                    uint16_t target_word = c.regs[Y_REG];

                    current_state = STATE_BUSY;
                    trigger_interrupt(c);

                    for(uint16_t offset = 0; offset < floppy::words_per_sector; offset++)
                    {
                        c.mem[(offset + target_word) % c.mem.size()] = floppy_opt.value().get_word(offset, reading_sector);
                    }

                    current_state = STATE_READY;
                    trigger_interrupt(c);
                }

                if(c.regs[A_REG] == 3)
                {
                    c.regs[B_REG] = 1;
                    uint16_t writing_sector = c.regs[X_REG];
                    uint16_t target_word = c.regs[Y_REG];

                    current_state = STATE_BUSY;
                    trigger_interrupt(c);

                    for(uint16_t offset = 0; offset < floppy::words_per_sector; offset++)
                    {
                        uint16_t word = c.mem[(offset + target_word) % c.mem.size()];

                        floppy_opt.value().set_word(offset, writing_sector, word);
                    }

                    current_state = STATE_READY;
                    trigger_interrupt(c);
                }
            }

            virtual hardware* clone() override
            {
                return new M35FD_simple(*this);
            }

            virtual void reset() override
            {
                *this = M35FD_simple();
            }
        };

        #if 0
        struct M35FD : hardware
        {
            using namespace M35FD_common;

            uint16_t current_state = STATE_READY;
            uint16_t last_error = ERROR_NONE;

            std::optional<floppy> floppy_opt;
            bool is_read_only = false;
            bool current_operation = false;

            bool interrupts_enabled = false;
            uint16_t interrupt_msg = 0;

            void trigger_interrupt()
            {

            }

            void set_read_only(bool _is_read_only)
            {
                is_read_only = _is_read_only;

                ///trigger transition
                if(current_state == STATE_READY)
                {
                    current_state = STATE_READY_WP;
                    trigger_interrupt();
                }
            }

            void insert_floppy(const floppy& flop)
            {
                floppy_opt = flop;

                if(is_read_only)
                    current_state = STATE_READY_WP;
                else
                    current_state = STATE_READY;

                trigger_interrupt();
            }

            void eject_floppy()
            {
                floppy_opt = std::nullopt;

                current_state = STATE_NO_MEDIA;

                trigger_interrupt();
            }

            std::optional<uint16_t> reading_sector;
            std::optional<uint16_t> writing_sector;
            std::optional<uint16_t> target_word;

            /*uint16_t get_current_status()
            {
                if(!floppy_opt.has_value())
                    return STATE_NO_MEDIA;

                if(current_operation)
                    return STATE_BUSY;

                if(is_read_only)
                    return STATE_READY_WP;

                return STATE_READY;
            }*/

            M35FD()
            {
                hardware_id = 0x4fd524c5;
                hardware_version = 0x000b;
                manufacturer_id = 0x1eb37e91;
            }

            constexpr virtual void interrupt2(std::span<hardware*> all_hardware, world_base* state, CPU& c)
            {
                if(c.regs[A_REG] == 0)
                {
                    c.regs[B_REG] = current_state;
                    c_regs[C_REG] = last_error;
                    last_error = ERROR_NONE;
                }

                if(c.regs[A_REG] == 1)
                {
                    interrupts_enabled = c.regs[X_REG] != 0;
                    interrupt_msg = c.regs[X_REG];
                }

                if(c.regs[A_REG] == 2)
                {
                    if(current_state == STATE_READY || current_state == STATE_READY_WP)
                    {
                        c.regs[B_REG] = 1;
                        reading_sector = c.regs[X_REG];
                        target_word = c.regs[Y_REG];
                    }
                    else
                    {
                        c.regs[B_REG] = 0;

                        if(current_state == STATE_BUSY)
                        {
                            last_error = ERROR_BUSY;
                            trigger_interrupt();
                        }

                        if(current_state == STATE_NO_MEDIA)
                        {
                            last_error = ERROR_NO_MEDIA;
                            trigger_interrupt();
                        }
                    }
                }

                if(c.regs[A_REG] == 3)
                {
                    if(current_state == STATE_READY)
                    {
                        c.regs[B_REG] = 1;
                        writing_sector = c.regs[X_REG];
                        target_word = c.regs[Y_REG];
                    }
                    else
                    {
                        c.regs[B_REG] = 0;

                        if(current_state == STATE_READY_WP)
                        {
                            last_error = ERROR_PROTECTED;
                            trigger_interrupt();
                        }

                        if(current_state == STATE_BUSY)
                        {
                            last_error = ERROR_BUSY;
                            trigger_interrupt();
                        }

                        if(current_state == STATE_NO_MEDIA)
                        {
                            last_error = ERROR_NO_MEDIA;
                            trigger_interrupt();
                        }
                    }
                }
            }

            virtual hardware* clone() override
            {
                return new M35FD(*this);
            }

            virtual void reset() override
            {
                *this = M35FD();
            }
        };
        #endif
    }
}

#endif // HARDWARE_M35FD_HPP_INCLUDED
