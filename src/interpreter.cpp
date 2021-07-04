#include<interpreter.h>
#include<display/display.h>
#include<chrono>

interpreter_t::interpreter_t(){
    //  memory callbacks.
    mem.unbind_bootrom_callbk = [&](){
        main_window::handshake([&](){
            main_window::bind(*this);
        }, !paused);
    };
    mem.read_breakpoint_callbk = [&](uint16_t adr){
        main_window::handshake([&](){
            paused = true;
            main_window::on_pause();
        }, !paused);
    };
    mem.write_breakpoint_callbk = [&](uint16_t adr, uint8_t val){
        main_window::handshake([&](){
            paused = true;
            main_window::on_pause();
        }, !paused);
    };
    //  cpu callbacks.
    cpu.code_breakpoints_callbk = [&](uint16_t adr, uint8_t opc, uint16_t imm){
        main_window::handshake([&](){
            paused = true;
            main_window::on_pause();
        }, !paused);
    };
    cpu.enter_call_callbk = [&](uint16_t p_pc, uint16_t c_pc){
        main_window::handshake([&](){
            call_deque.push_back({p_pc, c_pc});
        }, !paused);
    };
    cpu.ret_from_call_callbk = [&](){
        main_window::handshake([&](){
            call_deque.pop_back();
        }, !paused);
    };
    cpu.instruction_execute_callbk = [&](uint16_t adr, const std::string& mnemonic){
        main_window::handshake([&](){
            recent_instr_deque.push_front({adr, mnemonic});
            if(recent_instr_deque.size() > 15){
                recent_instr_deque.pop_back();
            }
        }, !paused);
    };
    //  tmp
    cpu.code_breakpoints[0x101] = true;
}

void interpreter_t::update(){
    //using namespace std::chrono;
    //auto time = high_resolution_clock::now();
    if(!paused){
        cpu.tick_step(0);
    } 
    if(should_step){
        should_step = false;
        cpu.tick_step(0);
    }
    //fps = 1.0_n/std::chrono::duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now()-time).count();
}