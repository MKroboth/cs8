//
// Created by mkr on 7/24/21.
//

#ifndef CS8_BUS_HXX
#define CS8_BUS_HXX
#include <cstdint>
#include <memory>

using bus_size_8 = uint8_t;
using bus_size_16 = uint16_t;
using device_id = uint8_t;

constexpr device_id BUS_UNOWNED = 0;
enum class RW {
    Read, Write, Off
};

template<class T, typename Data, typename Address>
concept BusLike = requires(T m) {
    { m.get_data()} -> std::same_as<Data>;
    {m.get_address()} -> std::same_as<Address>;
    {m.get_mode()} -> std::same_as<RW>;
    {m.get_bus_owner()} -> std::same_as<device_id>;
};

template<typename Data, typename Address, device_id ID, BusLike<Data, Address> Bus>
class BusDevice {
public:
    static constexpr device_id id = ID;
    std::weak_ptr<Bus> bus;
protected:
    device_id get_bus_owner() {
        if(auto b = bus.lock()) return b->get_bus_owner();
        else throw std::runtime_error("Lock failed");
    }
    auto get_bus_mode() {
        if(auto b = bus.lock()) return b->get_mode();
        else throw std::runtime_error("Lock failed");
    }
    auto get_bus_address() {
        if(auto b = bus.lock()) return b->get_address();
        else throw std::runtime_error("Lock failed");
    }
    auto get_bus_data() {
        if(auto b = bus.lock()) return b->get_data();
        else throw std::runtime_error("Lock failed");
    }
    void set_bus_data(Data value) {
        if(auto b = bus.lock()) b->set_data(value);
        else throw std::runtime_error("Lock failed");
    }
    void set_bus_address(Address value) {
        if(auto b = bus.lock()) b->set_address(value);
        else throw std::runtime_error("Lock failed");
    }
    void set_bus_mode(RW value) {
        if(auto b = bus.lock()) b->set_mode(value);
        else throw std::runtime_error("Lock failed");
    }
    void set_bus_owner(device_id value) {
        if(auto b = bus.lock()) b->set_bus_owner(value);
        else throw std::runtime_error("Lock failed");
    }

    void own_bus() {
        if(auto b = bus.lock()) b->set_bus_owner(ID);
        else throw std::runtime_error("Lock failed");
    }
    void disown_bus() {
        if(auto b = bus.lock()) b->set_bus_owner(BUS_UNOWNED);
        else throw std::runtime_error("Lock failed");
    }
};


template<typename Data, typename Address>
struct Bus {

private:
    Data data {0};
    Address address {0};
    RW mode { RW::Off };
    device_id bus_owner { BUS_UNOWNED };
public:
    using DataType = Data;
    using AddressType = Address;
    Data get_data() const {
        return data;
    }

    Address get_address() const {
        return address;
    }

    [[nodiscard]] RW get_mode() const {
        return mode;
    }

    [[nodiscard]] device_id get_bus_owner() const {
        return bus_owner;
    }

    void set_data(Data value) {
        data = value;
    }

    void set_address(Address value) {
        address = value;
    }

    void set_mode(RW value) {
        mode = value;
    }

    void set_bus_owner(device_id value) {
        bus_owner = value;
    }

};

template<typename Data, typename Address, device_id ID, BusLike<Data, Address> Bus>
void connect_bus(std::shared_ptr<Bus> bus, BusDevice<Data, Address, ID, Bus>& dev) {
    dev.bus = bus;
}
#endif //CS8_BUS_HXX
