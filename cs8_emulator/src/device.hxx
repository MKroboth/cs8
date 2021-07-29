//
// Created by mkr on 7/24/21.
//

#ifndef CS8_DEVICE_HXX
#define CS8_DEVICE_HXX


struct Device {
    virtual void init() {}
    virtual void simulate() = 0;
};


#endif //CS8_DEVICE_HXX
