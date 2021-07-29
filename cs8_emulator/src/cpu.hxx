//
// Created by mkr on 7/24/21.
//

#ifndef CS8_CPU_HXX
#define CS8_CPU_HXX

#include <bitset>
#include <iostream>
#include "bus.hxx"
#include "device.hxx"

constexpr size_t CPU_ID = 1;
template<typename Data, typename Address, BusLike<Data, Address> Bus>
class CPU : public BusDevice<Data, Address, CPU_ID, Bus>, public Device {
    using register_type = int16_t;

    register_type
        rdst,
        rsc0,
        rsc1,
        ridx,
        rtmp,
        rsp0,
        rsp1,
        rip,
        rS0,
        rS1,
        rS2,
        rS3,
        rS4,
        rS5,
        rln,
        rCNT, // Loop counter
        rBSE,
        rtmp2; // Base address

    std::array<register_type*, 0x10> registers {
            &rdst,
            &rsc0,
            &rsc1,
            &ridx,
            &rtmp,
            &rsp0,
            &rsp1,
            &rS0,
            &rS1,
            &rS2,
            &rS3,
            &rS4,
            &rS5,
            &rln,
            &rCNT,
            &rBSE
    };

    uint8_t rOP;
    uint8_t rR1, rR0;
    uint16_t rAddress;
    uint16_t rValue;

    enum class Opcode {
        Unknown = -1,
        LoadImm = 0,
        LoadDirect = 1,
        StoreDirect = 2,
        LoadIndexed = 3,
        StoreIndexed = 4,
        TransferRegister = 5,
        Push0 = 6,
        Push1 = 7,
        Pop0 = 8,
        Pop1 = 9,
        Add = 0xA,
        Sub = 0xB,
        Mul = 0xC,
        DivMod = 0xD,
        Nand = 0xE,
        Extended = 0xF
    } opcode {Opcode::Unknown};

    enum class Phase {
        Init, Fetch0, Fetch1,
        Decode, GetData0, GetData1, GetData2, GetData3,
        Prepare, Load0, Load1, Execute, Store0, Store1,
        Halted
    } cpu_phase = Phase::Init;

    static constexpr const char* to_string(Phase phase) {
        switch (phase) {
            case Phase::Init: return "Init";
            case Phase::Fetch0: return "Fetch0";
            case Phase::Fetch1: return "Fetch1";
            case Phase::Decode: return "Decode";
            case Phase::GetData0: return "GetData0";
            case Phase::GetData1: return "GetData1";
            case Phase::GetData2: return "GetData2";
            case Phase::GetData3: return "GetData3";
            case Phase::Prepare: return "Prepare";
            case Phase::Load0: return "Load0";
            case Phase::Load1: return "Load1";
            case Phase::Execute: return "Execute";
            case Phase::Store0: return "Store0";
            case Phase::Store1: return "Store1";
        }

        return "";//throw std::runtime_error("Invalid state");
    }

public:
    void simulate() override {
       switch (cpu_phase) {
           case Phase::Init: {
               rip = 0;
               cpu_phase = Phase::Fetch0;
           } break;
           case Phase::Fetch0: {
               this->own_bus();
               this->set_bus_mode(RW::Read);
               this->set_bus_address(rip++);
               cpu_phase = Phase::Fetch1;
           } break;
           case Phase::Fetch1: {
               rOP = this->get_bus_data();
               this->set_bus_mode(RW::Off);
               this->disown_bus();
               cpu_phase = Phase::Decode;
           } break;
           case Phase::Decode: {
               // rOP = 0b00000000
               //         1111XXXX
               auto opcode = rOP & 0x0F;
               this->opcode = static_cast<Opcode>(opcode);
               this->rR0 = (rOP ) >> 4;

               if(opcode == 0 || opcode == 1 || opcode == 2) {
                   cpu_phase = Phase::GetData0;
               } else if(opcode == 5) {
                   cpu_phase = Phase::GetData2;
               } else {
                   cpu_phase = Phase::Prepare;
               }

           } break;
           case Phase::GetData0: {
               this->own_bus();
               this->set_bus_mode(RW::Read);
               this->set_bus_address(rip++);
               cpu_phase = Phase::GetData1;
           } break;
           case Phase::GetData1: {
               rR0 = 0xFF & this->get_bus_data();
               this->set_bus_mode(RW::Off);
               cpu_phase = Phase::GetData2;
           } break;
           case Phase::GetData2: {

               this->own_bus();
               this->set_bus_mode(RW::Read);
               this->set_bus_address(rip++);
               cpu_phase = Phase::GetData3;
           } break;
           case Phase::GetData3: {
               rR1 = 0xFF & this->get_bus_data();
               this->set_bus_mode(RW::Off);
               this->disown_bus();
               cpu_phase = Phase::Prepare;
           } break;
           case Phase::Prepare: {
               rAddress = rR1 | (((unsigned)(rR0)) << 8);
               rValue = rAddress;
               rR0 = rR0 & 0x0F;
               rR1 = rR1 & 0x0F;

               if(opcode == Opcode::LoadDirect || opcode == Opcode::LoadIndexed || opcode == Opcode::Pop0 || opcode == Opcode::Pop1) {
                   cpu_phase = Phase::Load0;
               } else {
                   cpu_phase = Phase::Execute;
               }

           } break;
           case Phase::Load0: {
               this->own_bus();
               this->set_bus_mode(RW::Read);

               switch (opcode) {
                   case Opcode::LoadDirect:
                       this->set_bus_address(rAddress);
                       break;

                   case Opcode::LoadIndexed:
                       this->set_bus_address(((unsigned) (rBSE)) + ridx);
                       break;
                   case Opcode::Pop0:
                       this->set_bus_address((unsigned) rsp0);
                       break;
                   case Opcode::Pop1:
                       this->set_bus_address((unsigned) rsp1);
                       break;
               }

               cpu_phase = Phase::Load1;
           } break;
               case Phase::Load1: {

                   rValue = this->get_bus_data();
                   this->set_bus_mode(RW::Off);

                           cpu_phase = Phase::Execute;
                   }
                   case Phase::Execute: {

               switch (opcode) {
                   case Opcode::LoadImm: {
                       rR0 = 4;
                   }
                   case Opcode::LoadIndexed:
                   case Opcode::Pop0:
                   case Opcode::Pop1:
                   case Opcode::LoadDirect: {
                       rtmp2 = rtmp;
                       rtmp = rValue;
                   } break;
                   case Opcode::Push0:
                   case Opcode::Push1:
                   case Opcode::StoreDirect: {
                       rR0 = 4;
                   }
                   case Opcode::StoreIndexed: {
                       cpu_phase = Phase::Store0;
                   } return;
                   case Opcode::TransferRegister:
                       if(rR1 == 4) rtmp2 = rtmp;
                       *registers[rR1] = *registers[rR0];
                       break;
                   case Opcode::Add:
                       rdst = rsc0 + rsc1;
                       break;
                       case Opcode::Sub:
                           rdst = rsc0 - rsc1;
                   break; case Opcode::Mul:
                       rdst = rsc0 * rsc1;
                   break; case Opcode::DivMod:
                       rdst = rsc0 / rsc1;
                       rtmp2 = rtmp;
                       rtmp = rsc0 % rsc1;
                   break; case Opcode::Nand:
                       rdst = ~(rsc0 & rsc1);
                   break; case Opcode::Extended:
                       switch (rR0) {
                           case 0x00:
                               if(rCNT <= 0) {
                                   rln = rip;
                                   rip = rtmp;
                               }
                               break;
                           case 0x01:
                               if(rtmp == -1) {
                                   cpu_phase = Phase::Halted;
                                   return;
                               } else {
                                   rln = rip;
                                   rip = rtmp;
                               }
                               break;
                           case 0x02:
                           {
                               auto back = rtmp;
                               rtmp = rtmp2;
                               rtmp2 = back;
                           } break;
                       }
                       break;
               }

               cpu_phase = Phase::Fetch0;
           }
           case Phase::Store0: {
               this->set_bus_mode(RW::Write);
               switch (opcode) {
                   case Opcode::Push0:
                       this->set_bus_address((unsigned) rsp0);
                       break;
                   case Opcode::Push1:
                       this->set_bus_address((unsigned) rsp1);
                       break;
                   case Opcode::StoreDirect:
                       this->set_bus_address((unsigned) rAddress);
                       break;
                   case Opcode::StoreIndexed:
                       this->set_bus_address(((unsigned) rAddress) + ridx);
                       break;
               }
               switch (opcode) {
                   case Opcode::StoreDirect:
                   case Opcode::StoreIndexed:
                   case Opcode::Push0:
                   case Opcode::Push1:
                       this->set_bus_data(*registers[rR0]);
                       break;
               }
               cpu_phase = Phase::Store1;
           }break;
           case Phase::Store1: {
               this->set_bus_mode(RW::Off);
               this->set_bus_address(0);
               this->set_bus_data(0);
               cpu_phase = Phase::Fetch0;
           }
           case Phase::Halted: break;
       }
   }

   bool is_running() {
        return cpu_phase != Phase::Halted;
    }
};


#endif //CS8_CPU_HXX
