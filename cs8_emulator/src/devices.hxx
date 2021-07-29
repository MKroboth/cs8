//
// Created by mkr on 7/24/21.
//

#ifndef CS8_DEVICES_HXX
#define CS8_DEVICES_HXX
#include "bus.hxx"
#include "list"
#include "device.hxx"

struct Devices {
    std::list<std::shared_ptr<Device>> devices;

    void simulate() {
        for(auto& dev : devices) {
            dev->simulate();
        }
    };
};


#endif //CS8_DEVICES_HXX
