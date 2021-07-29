//
// Created by mkr on 7/24/21.
//

#ifndef CS8_MEMORY_HXX
#define CS8_MEMORY_HXX

#include <iostream>
#include <ios>
#include <functional>
#include "bus.hxx"
#include "device.hxx"

template<size_t Size, typename AllocUnit, typename Address, Address Begin, Address End, size_t ID, BusLike<AllocUnit, Address> Bus>
class Memory : public Device, public BusDevice<AllocUnit, Address, ID, Bus> {
public:
    using BufferType = std::array<AllocUnit, Size>;
private:
    BufferType memory_buffer;

public:
    using AllocationUnit = AllocUnit;
    static constexpr Address AddressBegin = Begin;
    static constexpr Address AddressEnd = End;
    static constexpr size_t DeviceID = ID;

    void modify(std::function<void (BufferType&)> const& f) {
        f(memory_buffer);
    }
    void simulate() override {
        if (this->get_bus_mode() == RW::Read ||
            this->get_bus_mode() == RW::Write) {
            if (this->get_bus_address() >= Begin &&
                this->get_bus_address() <= End) {
              /*  std::cerr << "Activated by "
                          << ((this->get_bus_mode() == RW::Write) ? "Write"
                                                                  : "Read")
                          << " from "
                          << std::hex << this->get_bus_address() << std::dec
                          << '\n';*/
                auto const addr = this->get_bus_address() - Begin;

                if (this->get_bus_mode() == RW::Read) {
                    this->set_bus_data(memory_buffer.at(addr));
                } else if (this->get_bus_mode() == RW::Write) {
                    memory_buffer.at(addr) = this->get_bus_data();
                }
            }
        }
    }
};



#endif //CS8_MEMORY_HXX
