#pragma once
#include<core/instructions.h>
#include<stdexcept>

enum class BIT:uint8_t{ B0=1,B1=2,B2=4,B3=8,B4=16,B5=32,B6=64,B7=128 };
enum class RST_VEC{ _00=0x00,_10=0x10,_20=0x20,_30=0x30,_08=0x08,_18=0x18,_28=0x28,_38=0x38 };

namespace instr_defs{
    /*
        functions marked with a '__' prefix are not meant to be used outside of the namespace.
    */
    enum BIT_VAL { SET=true,UNSET=false };
    template<RI8 reg>__always_inline auto& get_reg(cfa arg){ return arg.cpu.regs.get<reg>(); }
    template<RI16 reg>__always_inline auto& get_reg(cfa arg){ return arg.cpu.regs.get<reg>(); }
    template<FI flag>
    __always_inline bool get_flag(cfa arg){ return arg.cpu.regs.get_flag<flag>(); }
    template<FI flag>
    __always_inline void set_flag(cfa arg, bool v) { arg.cpu.regs.set_flag<flag>(v); }
    //  invalid instruction function
    inline void invalid_instr(cfa arg){
        throw std::runtime_error("tried to execute an invalid instruction.");
    }
    //  CB
    //  mostly a reserved function. cb prefixed instructions are put into another array than non-prefixed ones.
    inline void cb(cfa arg){}
    //  ADD
    __always_inline void __add(cfa arg, uint8_t val){
        auto tmp = get_reg<RI::A>(arg); //  caches the value of the A register for later comparison.
        get_reg<RI::A>(arg)+=val;
        set_flag<FI::Z>(arg,!get_reg<RI::A>(arg));
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,
            (tmp&0x0F)+(val&0x0F)>0x0F);
        set_flag<FI::C>(arg,
            tmp+val>0xFF);
    }
    template<RI8 src> void add_a_r8(cfa arg){
        __add(arg, get_reg<src>(arg));
    }
    inline void add_a_pointer_hl(cfa arg){
        size_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __add(arg, val);
    }
    inline void add_a_n8(cfa arg){
        __add(arg, arg.cpu.instr_info.imm8);
    }
    //  ADC
    template<RI8 src> void adc_a_r8(cfa arg){
        __add(arg, get_reg<src>(arg)+get_flag<FI::C>(arg));
    }
    inline void adc_pointer_hl(cfa arg){
        size_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __add(arg, val+get_flag<FI::C>(arg));
    }
    inline void adc_a_n8(cfa arg){
        __add(arg, arg.cpu.instr_info.imm8+get_flag<FI::C>(arg));
    }
    //  ADD 16
    template<RI16 src> void add_hl_r16(cfa arg){
        auto tmp = get_reg<RI::HL>(arg);
        get_reg<RI::HL>(arg)+=get_reg<src>(arg);
        set_flag<FI::N>(arg,UNSET);
        tmp<<=8;
        auto src_tmp = get_reg<src>(arg)>>8;
        set_flag<FI::H>(arg,(tmp&0x0F)+(src_tmp&0x0F)>0x0F);
        set_flag<FI::C>(arg,tmp+src_tmp>0xFF); 
    }
    inline void add_hl_sp(cfa arg){
        add_hl_r16<RI16::SP>(arg);
    }
    inline void add_sp_e8(cfa arg){
        uint8_t tmp = get_reg<RI::SP>(arg);
        get_reg<RI::SP>(arg)+=(int8_t)arg.cpu.instr_info.imm8;
        set_flag<FI::Z>(arg,UNSET);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,
            (tmp&0x0F)+(arg.cpu.instr_info.imm8&0x0F)>0x0F);
        set_flag<FI::C>(arg,
            tmp+arg.cpu.instr_info.imm8>0xFF);
    }
    //  AND
    __always_inline void __and(cfa arg, uint8_t val){
        auto& tmp = (get_reg<RI::A>(arg)&=val);
        set_flag<FI::Z>(arg,!tmp);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,SET);
        set_flag<FI::C>(arg,UNSET);
    }
    template<RI8 src> void and_a_r8(cfa arg){
        __and(arg,get_reg<src>(arg));
    }
    inline void and_a_pointer_hl(cfa arg){
        size_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __and(arg,val);
    }
    inline void and_a_n8(cfa arg){
        __and(arg,arg.cpu.instr_info.imm8);
    }
    //  BIT
    template<BIT bit> void __bit(cfa arg, uint8_t reg){
        set_flag<FI::Z>(arg,!(reg&(uint8_t)bit));
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,SET);
    }
    template<BIT bit,RI8 src> void bit_u3_r8(cfa arg){
        __bit<bit>(arg,get_reg<src>(arg));
    }
    template<BIT bit> void bit_u3_pointer_hl(cfa arg){
        size_t tmp = arg.memory.read(get_reg<RI::HL>(arg));
        __bit<bit>(arg,tmp);
    }
    //  CCF
    inline void ccf(cfa arg){
        set_flag<FI::C>(arg,!get_flag<FI::C>(arg));
    }
    //  CP
    __always_inline void __cp(cfa arg, uint8_t cmp){
        auto tmp = get_reg<RI::A>(arg);
        set_flag<FI::Z>(arg,!(tmp-cmp));
        set_flag<FI::N>(arg,SET);
        set_flag<FI::H>(arg,(tmp&0x0F)-(cmp&0x0F)<0);
        set_flag<FI::C>(arg,tmp-cmp>0);
    }
    template<RI8 src> void cp_a_r8(cfa arg){
        __cp(arg, get_reg<src>(arg));
    }
    inline void cp_a_pointer_hl(cfa arg){
        size_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __cp(arg, val);
    }
    inline void cp_a_n8(cfa arg){
        __cp(arg, arg.cpu.instr_info.imm8);
    }
    //  CPL
    inline void cpl(cfa arg){
        auto& tmp = get_reg<RI::A>(arg);
        tmp = ~tmp;
    }
    //  DAA
    inline void daa(cfa arg){
        //  for the future
        int unused;
    }
    //  DEC
    __always_inline void __dec(cfa arg, uint8_t& reg){
        set_flag<FI::H>(arg,(reg&0x0F)-1<0);
        --reg;
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,SET);
    }
    template<RI8 dest> void dec_r8(cfa arg){
        __dec(arg,get_reg<dest>(arg));
    }
    inline void dec_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __dec(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  DEC 16
    template<RI16 dest> void dec_r16(cfa arg){
        --get_reg<dest>(arg);
    }
    inline void dec_sp(cfa arg){
        dec_r16<RI::SP>(arg);
    }
    //  DI
    inline void di(cfa arg){
        //  for the future
        int unused;
    }
    //  EI
    inline void ei(cfa arg){
        //  for the future
        int unused;
    }
    //  HALT
    inline void halt(cfa arg){
        //  for the future
        int unused;
    }
    //  INC
    __always_inline void __inc(cfa arg, uint8_t& val){
        set_flag<FI::H>(arg,(val&0x0F)+1>0x0F);
        ++val;
        set_flag<FI::Z>(arg,!val);
        set_flag<FI::N>(arg,UNSET);
    }
    template<RI8 dest> void inc_r8(cfa arg){
        __inc(arg,get_reg<dest>(arg));
    }
    inline void inc_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __inc(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  INC r16
    template<RI16 dest> void inc_r16(cfa arg){
        ++get_reg<dest>(arg);
    }
    inline void inc_sp(cfa arg){
        inc_r16<RI::SP>(arg);
    }
    //  JP
    inline void jp_n16(cfa arg){
        get_reg<RI::PC>(arg)=arg.cpu.instr_info.imm16;
    }
    template<FI flag,BIT_VAL state> void jp_cc_n16(cfa arg){
        if((arg.did_branch = get_flag<flag>(arg)==state))
            jp_n16(arg);
    }
    inline void jp_hl(cfa arg){
        get_reg<RI::PC>(arg)=get_reg<RI::HL>(arg);
    }
    //  JR
    inline void jr_e8(cfa arg){
        get_reg<RI::PC>(arg)+=(int8_t)arg.cpu.instr_info.imm8;
    }
    template<FI flag, BIT_VAL state>
    inline void jr_cc_e8(cfa arg){
        if((arg.did_branch = get_flag<flag>(arg)==state))
            jr_e8(arg);
    }
    //  LD
    template<RI8 dest,RI8 src> void ld_r8_r8(cfa arg){
        get_reg<dest>(arg)=get_reg<src>(arg);
    }
    template<RI8 dest> void ld_r8_n8(cfa arg){
        get_reg<dest>(arg)=arg.cpu.instr_info.imm8;
    }
    template<RI16 dest> void ld_r16_n16(cfa arg){
        get_reg<dest>(arg)=arg.cpu.instr_info.imm16;
    }
    template<RI8 src> void ld_pointer_hl_r8(cfa arg){
        arg.memory.write(get_reg<RI::HL>(arg), get_reg<src>(arg));
    }
    inline void ld_pointer_hl_n8(cfa arg){
        arg.memory.write(get_reg<RI::HL>(arg), arg.cpu.instr_info.imm8);
    }
    template<RI8 dest> void ld_r8_pointer_hl(cfa arg){
        get_reg<dest>(arg)=arg.memory.read(get_reg<RI::HL>(arg));
    }
    template<RI16 dest_pointer> void ld_pointer_r16_a(cfa arg){
        arg.memory.write(get_reg<dest_pointer>(arg),get_reg<RI::A>(arg));
    }
    inline void ld_pointer_n16_a(cfa arg){
        arg.memory.write(arg.cpu.instr_info.imm16,get_reg<RI::A>(arg));
    }
    template<RI16 src_pointer> void ld_a_pointer_r16(cfa arg){
        get_reg<RI::A>(arg)=arg.memory.read(get_reg<src_pointer>(arg));
    }
    inline void ld_a_pointer_n16(cfa arg){
        get_reg<RI::A>(arg)=arg.cpu.instr_info.imm16;
    }
    inline void ld_pointer_hl_increment_a(cfa arg){
        arg.memory.write(get_reg<RI::HL>(arg)++,get_reg<RI::A>(arg));
    }
    inline void ld_pointer_hl_decrement_a(cfa arg){
        arg.memory.write(get_reg<RI::HL>(arg)--,get_reg<RI::A>(arg));
    }
    inline void ld_a_pointer_hl_increment(cfa arg){
        get_reg<RI::A>(arg)=arg.memory.read(get_reg<RI::HL>(arg)++);
    }
    inline void ld_a_pointer_hl_decrement(cfa arg){
        get_reg<RI::A>(arg)=arg.memory.read(get_reg<RI::HL>(arg)--);
    }
    inline void ld_sp_n16(cfa arg){
        get_reg<RI::SP>(arg)=arg.cpu.instr_info.imm16;
    }
    inline void ld_pointer_n16_sp(cfa arg){
        arg.memory.write(arg.cpu.instr_info.imm16, get_reg<RI::SP>(arg));
        arg.memory.write(arg.cpu.instr_info.imm16+1,get_reg<RI::SP>(arg)>>8);
    }
    inline void ld_hl_sp_e8(cfa arg){
        get_reg<RI::HL>(arg)+=get_reg<RI::SP>(arg)+(int8_t)arg.cpu.instr_info.imm8;
    }
    inline void ld_sp_hl(cfa arg){
        get_reg<RI::SP>(arg)=get_reg<RI::HL>(arg);
    }
    //  LDH
    inline void ldh_pointer_n8_a(cfa arg){
        arg.memory.write(0xFF00+arg.cpu.instr_info.imm8,get_reg<RI::A>(arg));
    }
    inline void ldh_pointer_c_a(cfa arg){
        arg.memory.write(0xFF00+get_reg<RI::C>(arg),get_reg<RI::A>(arg));
    }
    inline void ldh_a_pointer_n8(cfa arg){
        get_reg<RI::A>(arg)=arg.memory.read(0xFF00+arg.cpu.instr_info.imm8);
    }
    inline void ldh_a_pointer_c(cfa arg){
        get_reg<RI::A>(arg)=arg.memory.read(0xFF00+get_reg<RI::C>(arg));
    }
    //  NOP
    inline void nop(cfa arg){}
    //  OR
    __always_inline void __or(cfa arg, uint8_t val){
        auto tmp = (get_reg<RI::A>(arg)|=val);
        set_flag<FI::Z>(arg,!tmp);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
        set_flag<FI::C>(arg,UNSET);
    }
    template<RI8 dest> void or_a_r8(cfa arg){
        __or(arg,get_reg<dest>(arg));
    }
    inline void or_a_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __or(arg,val);
    }
    inline void or_a_n8(cfa arg){
        __or(arg,arg.cpu.instr_info.imm8);
    }
    //  PUSH
    template<RI16 src> void push_r16(cfa arg){
        uint16_t val = get_reg<src>(arg);
        auto& tmp = get_reg<RI::SP>(arg);
        arg.memory.write(--tmp,val>>8);
        arg.memory.write(--tmp,val);
    }
    //  POP
    template<RI16 dest> void pop_r16(cfa arg){
        uint16_t val = 0;
        auto& tmp = get_reg<RI::SP>(arg);
        val|=arg.memory.read(tmp++);
        val|=arg.memory.read(tmp++)<<8;
        get_reg<dest>(arg) = val;
    }
    //  CALL
    inline void call_n16(cfa arg){
        push_r16<RI::PC>(arg);
        get_reg<RI::PC>(arg)=arg.cpu.instr_info.imm16;
    }
    template<FI flag,BIT_VAL state> void call_cc_n16(cfa arg){
        if((arg.did_branch = get_flag<flag>(arg)==state))
            call_n16(arg);
    }
    //  RES
    template<BIT bit, RI8 dest> void res_u3_r8(cfa arg){
        get_reg<dest>(arg)^=((uint8_t)bit&get_reg<dest>(arg));
    } 
    template<BIT bit> void res_u3_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        val^=((uint8_t)bit&val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  RET
    inline void ret(cfa arg){
        pop_r16<RI::PC>(arg);
    }
    template<FI flag,BIT_VAL state> void ret_cc(cfa arg){
        if((arg.did_branch = get_flag<flag>(arg)==state))
            ret(arg);
    }
    inline void reti(cfa arg){
        ei(arg);
        ret(arg);
    }
    //  RL
    __always_inline void __rl(cfa arg, uint8_t& reg){
        bool carry = get_flag<FI::C>(arg);
        set_flag<FI::C>(arg,reg&0x80);
        reg<<=1;
        reg+=carry;
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
    }
    template<RI8 dest> void rl_r8(cfa arg){
        __rl(arg, get_reg<dest>(arg));
    }
    inline void rla(cfa arg){
        __rl(arg,get_reg<RI::A>(arg));
        set_flag<FI::Z>(arg,UNSET);
    }
    inline void rl_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __rl(arg, val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  RLC
    __always_inline void __rlc(cfa arg, uint8_t& reg){
        bool bit = reg&0x80;
        reg<<=1;
        reg+=bit;
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
        set_flag<FI::C>(arg,bit);
    }
    template<RI8 dest> void rlc_r8(cfa arg){
        __rlc(arg,get_reg<dest>(arg));
    }
    inline void rlca(cfa arg){
        __rlc(arg,get_reg<RI::A>(arg));
        set_flag<FI::Z>(arg,UNSET);
    }
    inline void rlc_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __rlc(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  RR
    __always_inline void __rr(cfa arg, uint8_t& reg){
        uint8_t carry = 0x80*get_flag<FI::C>(arg);
        set_flag<FI::C>(arg,reg&0b1);
        reg>>=1;
        reg+=carry;
        set_flag<FI::Z>(arg,reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
    }
    template<RI8 dest> void rr_r8(cfa arg){ 
        __rr(arg,get_reg<dest>(arg)); 
    }
    inline void rra(cfa arg){
        __rr(arg,get_reg<RI::A>(arg));
        set_flag<FI::Z>(arg,UNSET);
    }
    inline void rr_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __rr(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  RRC
    __always_inline void __rrc(cfa arg, uint8_t& reg){
        uint8_t bit = reg&0x80;
        set_flag<FI::C>(arg,reg&0b1);
        reg>>=1;
        reg+=bit;
        set_flag<FI::Z>(arg,reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
    }
    template<RI8 dest> void rrc_r8(cfa arg){
        __rrc(arg,get_reg<dest>(arg));
    }
    inline void rrca(cfa arg){
        __rrc(arg,get_reg<RI::A>(arg));
        set_flag<FI::Z>(arg,UNSET);
    }
    inline void rrc_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __rrc(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  RST
    template<RST_VEC adr> void rst(cfa arg){
        get_reg<RI::PC>(arg)=static_cast<uint16_t>(adr);
    }
    //  SCF
    inline void scf(cfa arg){
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
        set_flag<FI::C>(arg,SET);
    }
    //  SET
    template<BIT bit,RI8 dest> void set_u3_r8(cfa arg){
        get_reg<dest>(arg)|=(uint8_t)bit;
    }
    template<BIT bit> void set_u3_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        val|=(uint8_t)bit;
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  SLA
    __always_inline void __sla(cfa arg, uint8_t& reg){
        set_flag<FI::C>(arg,reg&0x80);
        reg<<=1;
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
    }
    template<RI8 dest> void sla_r8(cfa arg){
        __sla(arg,get_reg<dest>(arg));
    }
    inline void sla_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __sla(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  SRA
    __always_inline void __sra(cfa arg, uint8_t& reg){
        uint8_t bit = reg&0x80;
        set_flag<FI::C>(arg,reg&0b1);
        reg>>=1;
        reg|=bit;
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
    }
    template<RI8 dest> void sra_r8(cfa arg){
        __sra(arg,get_reg<dest>(arg));
    }
    inline void sra_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __sra(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  SRL
    __always_inline void __srl(cfa arg, uint8_t reg){
        set_flag<FI::C>(arg,reg&0b1);   
        reg>>=1;
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
    }
    template<RI8 dest> void srl_r8(cfa arg){
        __srl(arg,get_reg<dest>(arg));
    }
    inline void srl_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __srl(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  STOP
    inline void stop(cfa arg){
        //  for the future
        int unused;
    }
    //  SUB
    __always_inline void __sub(cfa arg, uint8_t val){
        auto& tmp = get_reg<RI::A>(arg);
        set_flag<FI::H>(arg,(tmp&0x0F)-(val&0x0F)<0);
        set_flag<FI::C>(arg,tmp-val<0);
        tmp-=val;
        set_flag<FI::Z>(arg,!tmp);
        set_flag<FI::N>(arg,SET);
    }
    template<RI8 src> void sub_a_r8(cfa arg){
        __sub(arg,get_reg<src>(arg));
    }
    inline void sub_a_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __sub(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    inline void sub_a_n8(cfa arg){
        __sub(arg,arg.cpu.instr_info.imm8);
    }
    //  SBC
    __always_inline void __sbc(cfa arg, uint8_t val){
        __sub(arg,val+get_flag<FI::C>(arg));
    }
    template<RI8 src> void sbc_a_r8(cfa arg){
        __sbc(arg,get_reg<src>(arg));
    }
    inline void sbc_a_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __sbc(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    inline void sbc_a_n8(cfa arg){
        __sub(arg,arg.cpu.instr_info.imm8);
    }
    //  SWAP
    __always_inline void __swap(cfa arg, uint8_t& reg){
        reg = (reg<<4)|(reg>>4);
        set_flag<FI::Z>(arg,!reg);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
        set_flag<FI::C>(arg,UNSET);
    }
    template<RI8 dest> void swap_r8(cfa arg){
        __swap(arg,get_reg<dest>(arg));
    }
    inline void swap_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __swap(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    //  XOR
    __always_inline void __xor(cfa arg, uint8_t reg){
        auto tmp = (get_reg<RI::A>(arg)^=reg);
        set_flag<FI::Z>(arg,!tmp);
        set_flag<FI::N>(arg,UNSET);
        set_flag<FI::H>(arg,UNSET);
        set_flag<FI::C>(arg,UNSET);
    }
    template<RI8 src> void xor_a_r8(cfa arg){
        __xor(arg,get_reg<src>(arg));
    }
    inline void xor_a_pointer_hl(cfa arg){
        uint8_t val = arg.memory.read(get_reg<RI::HL>(arg));
        __xor(arg,val);
        arg.memory.write(get_reg<RI::HL>(arg),val);
    }
    inline void xor_a_n8(cfa arg){
        __xor(arg,arg.cpu.instr_info.imm8);
    }
};