#include <cpu.hpp>

namespace narcissus {
    namespace cpu {

        h8_300::h8_300(std::array<std::uint8_t, ROM_SIZE>&& mem) 
            : er(), sp(), ccr(), pc(), memory(move(mem))
        {}

        void h8_300::reset_exception()
        {
            // load pc from memory[0] ~ memory[3]
            auto reset_addr = std::uint32_t(memory[0]) << 24;
            reset_addr |= std::uint32_t(memory[1]) << 16;
            reset_addr |= std::uint32_t(memory[2]) << 8;
            reset_addr |= std::uint32_t(memory[3]);

            pc = reset_addr;

            ccr.byte = 0b00000000;
            ccr.interrupt_mask = 1;
        }

        bool h8_300::cycle()
        {
            switch (detect_operation()) {
//                 case ADD_B_IMM: {
//                     auto rd = memory[pc] & 0x0f;
//                     auto imm = memory[pc + 1];
//                     if (!register_write_immediate(rd, imm, register_size::BYTE)) {
//                         return false;
//                     }

//                     pc += 2;
//                     break;
//                 }

//                 case ADD_B_R_R:
//                 {
//                     auto rs = (memory[pc + 1] & 0xf0) >> 4;
//                     auto rd = memory[pc + 1] & 0x0f;

//                     if(!register_write_register(rd, rs, register_size::BYTE)){
//                         return false;
//                     }

//                     pc += 2;
//                     break;        
//                 }

//                 case ADD_W_IMM:
//                 {
//                     auto rd = (memory[pc + 1] & 0xf);
//                     std::uint16_t imm = std::uint16_t(memory[pc + 2]) << 8;
//                     imm |= std::uint16_t(memory[pc + 3]);

//                     if(!register_write_immediate(rd, imm, register_size::WORD))
//                     {
//                         return false;
//                     }

//                     pc += 4;
//                     break;
//                 }

//                 case ADD_W_R_R:
//                 {
//                     auto rs = (memory[pc + 1] & 0xf0) >> 4;
//                     auto rd = (memory[pc + 1] & 0x0f);

//                     if(!register_write_register(rd, rs, register_size::WORD)){
//                         return false;
//                     }

//                     pc += 2;

//                     break;
//                 }


//                 case ADD_L_IMM:
//                 {
//                     std::uint8_t erd = memory[pc + 1] & 0x7;
//                     auto imm = std::uint32_t(memory[pc + 2]) << 24;
//                     imm |= std::uint32_t(memory[pc + 3]) << 16;
//                     imm |= std::uint32_t(memory[pc + 4]) << 8;
//                     imm |= std::uint32_t(memory[pc + 5]);

//                     if(!register_write_immediate(std::uint8_t(erd), imm, register_size::LONG)){
//                         return false;
//                     }

//                     pc += 6;
//                     break;
//                 }

//                 case ADD_L_R_R:
//                 {
//                     auto ers = (memory[pc + 1] & 0x70) >> 8;
//                     auto erd = memory[pc + 1] & 0x07;
                    
//                     if(!register_write_register(erd, ers, register_size::LONG)){
//                         return false;
//                     }

//                     pc += 2;
//                     break;
//                 }

                case SUB_B_R_R:
                {
                    auto rs = (memory[pc + 1]) >> 4;
                    auto rd = (memory[pc + 1]) & 0xf;

                    auto src_value = er[rs & 0x7].read(rs, register_size::BYTE);
                    auto dest_value = er[rd & 0x7].read(rd, register_size::BYTE);
                    dest_value -= src_value;

                    if(!register_write_immediate(rd, dest_value, register_size::BYTE))
                    {
                        return false;
                    }

                    pc += 2;
                    break;
                }

                case SUB_W_R_R:
                {
                    auto rs = memory[pc + 1] >> 0x4;
                    auto rd = memory[pc + 1] & 0x4;

                    auto src_value = er[rs & 0x7].read(rs, register_size::WORD);
                    auto dest_value = er[rd & 0x7].read(rd, register_size::WORD);

                    dest_value -= src_value;
                    if(!register_write_immediate(rd, dest_value, register_size::WORD))
                    {
                        return false;
                    }

                    pc += 2;
                    break;
                }

                case MOV_B_IMM:
                {
                    auto rd = memory[pc] & 0x0f;
                    auto imm = memory[pc + 1];

                    if(!register_write_immediate(rd, imm, register_size::BYTE)){
                        return false;
                    }
                    pc += 2;

                    break;
                }

                case MOV_B_R_IND:
                {
                    auto erd = (memory[pc + 1] & 0x70) >> 4;
                    auto rs = memory[pc + 1] & 0x8;

                    auto src_value = er[rs & 0x7].read(rs, register_size::BYTE);
                    auto dest_value = er[erd & 0x7].read(erd, register_size::LONG);
                    memory[dest_value] = src_value;

                    pc += 2;
                    break;
                }

                case MOV_B_R_IND_WITH_DIS_16:
                {
                    auto erd = (memory[pc + 1] >> 4) & 0x7;
                    auto rs = (memory[pc + 1]) & 0xf;
                    auto disp = (std::uint16_t)(memory[pc + 2]) << 8;
                    disp |= (std::uint16_t)(memory[pc + 3]);

                    auto src_value = er[rs & 0x7].read(rs, register_size::BYTE);
                    auto dest_addr = er[erd & 0x7].read(erd, register_size::LONG);

                    dest_addr += disp;
                    memory[dest_addr] = src_value;

                    pc += 4;
                    break;
                }

                case MOV_W_IMM:
                {
                    auto rd = memory[pc + 1] & 0x7;
                    auto imm = uint16_t(memory[pc + 2]) << 8;
                    imm |= uint16_t(memory[pc + 3]);

                    if(!register_write_immediate(rd, imm, register_size::WORD)){
                        return false;
                    }
                    pc += 4;
                    break;
                }

                case MOV_L_IMM:
                {
                    auto erd = memory[pc + 1] & 0x7;
                    auto imm = std::uint32_t(memory[pc + 2]) << 24;
                    imm |= std::uint32_t(memory[pc + 3]) << 16;
                    imm |= std::uint32_t(memory[pc + 4]) << 8;
                    imm |= std::uint32_t(memory[pc + 5]);

                    std::cout << imm << std::endl;
                    if(!register_write_immediate(erd, imm, register_size::LONG)){
                        return false;
                    }
                    pc += 6;
                    return true;
                }

                case MOV_L_R_R:
                {
                    auto ers = std::uint32_t(memory[pc + 1] & 0x70) >> 4;
                    auto erd = std::uint32_t(memory[pc + 1] & 0x07);

                    auto src_value = er[ers].er32;
                    er[erd].er32 = src_value;

                    pc += 2;
                    return true;
                }

                case MOV_L_R_IND:
                {
                    auto erd = (memory[pc + 3] & 0x70) >> 4;
                    auto ers = (memory[pc + 3] & 0x07);

                    auto erd_val = er[erd].er32;
                    auto ers_val = er[ers].er32;

                    erd_val -= 4;
//
                    memory[erd_val] = ers_val >> 24;
                    memory[erd_val + 1] = ers_val >> 16;
                    memory[erd_val + 2] = ers_val >> 8;
                    memory[erd_val + 3] = ers_val;

                    er[erd].er32 = erd_val;

                    pc += 4;

                    return true;
                }

                case MOV_L_R_IND_WITH_DIS_24:
                {
                    auto ers = memory[pc + 3] >> 4;
                    auto erd = memory[pc + 5] & 0x3;
                    auto disp = std::uint32_t(memory[pc + 7]) << 16;
                    disp |= std::uint32_t(memory[pc + 8]) << 8;
                    disp |= std::uint32_t(memory[pc + 9]);

                    auto addr = er[ers].er32 + disp;

                    std::cout << std::hex << ers << ":" 
                        << erd << ":" << disp << ":" << addr << std::endl;

                    er[erd].er32 = std::uint32_t(memory[addr]) << 24;
                    er[erd].er32 |= std::uint32_t(memory[addr + 1]) << 16;
                    er[erd].er32 |= std::uint32_t(memory[addr + 2]) << 8;
                    er[erd].er32 |= std::uint32_t(memory[addr + 3]);

                    pc += 10;
                    return true;
                }

                case JSR_ABS:
                {
                    auto abs = std::uint32_t(memory[pc + 1]) << 16;
                    abs |= std::uint32_t(memory[pc + 2]) << 8;
                    abs |= std::uint32_t(memory[pc + 3]);

                    memory[--sp] = (std::uint8_t)(pc & 0x0000ff);
                    memory[--sp] = (std::uint8_t)((pc >> 8) & 0x0000ff);
                    memory[--sp] = (std::uint8_t)((pc >> 16) & 0x0000ff);

                    pc = abs;
                    return true;
                }

                case EXTS_L:
                {
                    auto erd = memory[pc + 1] & 0x7;
                    auto val = er[erd].er32;
                    uint32_t sign = (uint32_t)(val & 0x00008000);
                    sign |= sign << 1; // 0x00018000;
                    sign |= sign << 1; // 0x00038000;
                    sign |= sign << 2; // 0x000f8000;
                    sign |= sign << 4; // 0x00ff8000;
                    sign |= sign << 8; // 0xffff8000;

                    er[erd].er32 = (val & 0x0008fff) | sign;

                    pc += 2;
                    return true;
                }

                case SHLL_L:
                {
                    auto erd = memory[pc + 1] & 0x7;
                    er[erd].er32 = er[erd].er32 << 1;

                    pc += 2;
                    return true;
                }


                //case operation::INVALID:
                default:

                    std::cout << "INVALID opecode: " << std::hex << "0x" << std::flush;
                    std::cout << std::setw(2) << std::setfill('0') 
                        << (std::uint16_t)(memory[pc]) << std::flush;
                    std::cout << std::setw(2) << std::setfill('0') 
                        << (std::uint16_t)(memory[pc+1]) << std::endl;

                    std::cout << "pc             : 0x" << std::setw(8) << std::setfill('0')
                        << (std::uint32_t)pc << std::endl;

                    return false;
            }

            return true;
        }

        operation h8_300::detect_operation()
        {
            std::uint8_t op = memory[pc];

            auto ah = op >> 4;
            auto al = op & 0x0f;

            auto op2 = memory[pc + 1];
            auto bh = op2 >> 4;
            auto bl = op2 & 0x0f;

            auto op3 = memory[pc + 2];
            auto ch = op3 >> 4;
            auto cl = op3 & 0x0f;

            switch (ah) { 

                case 0:
                    switch (al) {
                        case 1:
                            switch (bh) {
                                case 0:
                                    switch (bl) {
                                        case 0:
                                            switch (ch) {
                                                case 6:
                                                    switch (cl) {
                                                        case 0xd:
                                                            return operation::MOV_L_R_IND;
                                                        default:
                                                            return operation::INVALID;
                                                    }
                                                case 7:
                                                    switch (cl) {
                                                        case 8:
                                                        {
                                                            auto t = memory[pc + 5] >> 4;
                                                            switch (t) {
                                                                case 0x2:
                                                                    return operation::MOV_L_R_IND_WITH_DIS_24;
                                                                default:
                                                                    return operation::INVALID;
                                                            }
                                                        }
                                                        default:
                                                            return operation::INVALID;
                                                    }
                                                default:
                                                    return operation::INVALID;
                                            }       
                                        default:
                                            return operation::INVALID;
                                    }
                                default:
                                    return operation::INVALID;
                            }
                        case 8:
//                             return operation::ADD_B_R_R;
                            return operation::INVALID;
                        case 9:
//                             return operation::ADD_W_R_R;
                            return operation::INVALID;
                        case 0xa:
                            switch (bh) {
                                case 0x8:
                                case 0x9:
                                case 0xa:
                                case 0xb:
                                case 0xc:
                                case 0xd:
                                case 0xe:
                                case 0xf:
//                                     return operation::ADD_L_R_R;

                                default:
                                    return operation::INVALID;
                            }
                        case 0xf:
                            return operation::MOV_L_R_R;
                        default:
                            return operation::INVALID;
                    }
                case 1:
                    switch (al) {
                        case 0:
                            switch (bh) {
                                case 3:
                                    return operation::SHLL_L; 
                                default:
                                    return operation::INVALID;
                            }
                        case 7:
                            switch (bh) {
                                case 0xf:
                                    return operation::EXTS_L;

                                default:
                                    return operation::INVALID;
                            }
                        case 8:
                            return operation::SUB_B_R_R;

                        case 9:
                            return operation::SUB_W_R_R;

                        default:
                            return operation::INVALID;
                    }

                case 5:
                    switch (al) {
                        case 0xe:
                            return operation::JSR_ABS;

                        default:
                            return operation::INVALID;
                    }

                case 6:
                    switch (al) {
                        case 0xe:
                            return operation::MOV_B_R_IND_WITH_DIS_16;

                        case 0x8:
                            return operation::MOV_B_R_IND;
                        
                        default:
                            return operation::INVALID;
                    }

                case 7:
                    switch(al) {
                        case 9:
                            switch (bh) {
                                case 1:
//                                     return operation::ADD_W_IMM;

                                case 0:
                                    return operation::MOV_W_IMM;

                                default:
                                    return INVALID;
                            }

                        case 0xa:
                            switch(bh) {
                                case 0:
                                    return operation::MOV_L_IMM;
                                case 1:
//                                     return operation::ADD_L_IMM;
                                default:
                                    return INVALID;
                            }
                        default:
                            return INVALID;
                    }

                case 8:
//                     return operation::ADD_B_IMM;
                    return operation::INVALID;

                case 0xf:
                    return operation::MOV_B_IMM;

                default:
                    return operation::INVALID;
            }
        }

        bool h8_300::register_write_immediate(std::uint8_t destination,
                std::uint32_t immediate,
                register_size size)
        {

            switch (size) {
                case BYTE:
                    if (immediate > 0xff) {
                        return false;
                    }
                    break;

                case WORD:
                    if(immediate > 0xffff){
                        return false;
                    }
                    break;
                case LONG:
                    break;
            }

            er[destination & 0x7].write(destination, immediate, size);
            return true;
        }

        bool h8_300::register_write_register(std::uint8_t destination,
                std::uint8_t source,
                register_size size)
        {
            if(destination > 0xf || source > 0xf) {
                return false;
            }

            auto tmp = er[source & 0x7].read(source, size);
            er[destination & 0x7].write(destination, tmp, size);

//             switch (size) {
//                 case register_size::BYTE: 
//                     {

//                         auto tmp = er[source & 0x7].read(source, register_size::BYTE);
//                         er[destination & 0x7].write(destination, tmp, register_size::BYTE);
//                         break;
//                     }
//                 case register_size::WORD:
//                     {
//                         uint16_t tmp = er[source & 0x7].read(source, register_size::WORD);
//                         er[destination & 0x7].write(destination, tmp, register_size::BYTE) ;
//                         
//                         break;
//                     }
//                 case register_size::LONG:
//                     break;

//             }
            return true;
        }


    }  // namespace cpu
}  // namespace narcissus
