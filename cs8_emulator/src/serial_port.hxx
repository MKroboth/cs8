//
// Created by mkr on 7/28/21.
//

#ifndef CS8_SERIAL_PORT_HXX
#define CS8_SERIAL_PORT_HXX
#include <cstdint>
#include <iostream>
#include "bus.hxx"
#include "device.hxx"

template<size_t Size, typename AllocUnit, typename Address, Address Begin, Address End, size_t ID, BusLike<AllocUnit, Address> Bus>
class SerialPort : public Device, public BusDevice<AllocUnit, Address, ID, Bus> {

public:
    using AllocationUnit = AllocUnit;
    static constexpr Address AddressBegin = Begin;
    static constexpr Address AddressEnd = End;
    static constexpr size_t DeviceID = ID;

    void simulate() override {
        if (this->get_bus_mode() == RW::Read ||
            this->get_bus_mode() == RW::Write) {
            if (this->get_bus_address() >= Begin &&
                this->get_bus_address() <= End) {
                auto const address = Begin - this->get_bus_address();

                if (this->get_bus_mode() == RW::Read) {
                    switch (address) {
                        case 0:
                            this->set_bus_data((char)std::getchar());
                            break;
                            default:
                                break;
                    }
                } else if (this->get_bus_mode() == RW::Write) {
                    switch (address) {
                        case 0:
                          //  std::cerr << "Triggered Address: " << address << " Data: " << std::hex << this->get_bus_data() << std::dec << '\n';
                            {
                                char data = this->get_bus_data();
                                printf("%c", data);
                                fflush(stdout);
                            }//putchar(this->get_bus_data()>>8);
                            break;
                            default:
                                break;
                    }
                }
            }
        }
    }
};
#endif //CS8_SERIAL_PORT_HXX
