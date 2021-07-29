//
// Created by mkr on 7/24/21.
//

#include <cassert>
#include "devices.hxx"
#include "bus.hxx"
#include "cpu.hxx"
#include "memory.hxx"
#include "badasm.hxx"
#include "serial_port.hxx"
#include <elfio/elfio.hpp>
#include <filesystem>

using BusType = Bus<bus_size_16, bus_size_16>;
using CPUType = CPU<BusType::DataType, BusType::AddressType, BusType>;
using EmulatedMemory = Memory<0x1FFF, BusType::DataType, BusType::AddressType, 0x0000, 0x1FFF, 0xA0, BusType>;
using EmulatedSerialPort = SerialPort<0x1FFF, BusType::DataType, BusType::AddressType, 0x2000, 0x2001, 0xA1, BusType>;

/**
 * Load the specified file into memory
 * @param file a path to an elf file
 * @param buffer the targeted memory
 */
void initialize_memory(std::filesystem::path const& file, EmulatedMemory::BufferType& buffer) {
    ELFIO::elfio reader;
    reader.load(file);

    for(auto const segment : reader.segments) {
        auto const* data = segment->get_data();
        auto const size = segment->get_memory_size();
        auto const address = segment->get_virtual_address();
        std::copy(data, data+size, buffer.begin()+address);
    }
}

int main(int argc, const char* argv[]) {
    if(argc < 2) return -1;
    std::filesystem::path program_file(argv[1]);
    std::shared_ptr<BusType> bus = std::make_shared<BusType>();
    std::shared_ptr<CPUType> cpu =
            std::make_shared<CPUType>();
    std::shared_ptr<EmulatedMemory> memory =
            std::make_shared<EmulatedMemory>();
    std::shared_ptr<EmulatedSerialPort> serialPort = std::make_shared<EmulatedSerialPort>();

    auto f = std::bind_front(initialize_memory, program_file);
    memory->modify(f);


    Devices devices;
    devices.devices.push_front(serialPort);
    devices.devices.push_front(memory);
    devices.devices.push_front(cpu);

    for(auto& device : devices.devices) {
        device->init();
    }


    connect_bus(bus, *cpu);
    connect_bus(bus, *memory);
    connect_bus(bus, *serialPort);

    while (cpu->is_running()) {
        devices.simulate();
    }
}