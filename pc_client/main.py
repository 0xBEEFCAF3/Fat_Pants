#!/usr/bin/python3
import usb.core
import usb.util
import sys


if __name__ == '__main__':
    print("==================== Welcome to Fat Pants ====================")
    # find USB devices
    dev = usb.core.find(find_all=True)
    print(dev)
    # loop through devices, printing vendor and product ids in decimal and hex
    for cfg in dev:
        print('Decimal VendorID=' + str(cfg.idVendor) + ' & ProductID=' + str(cfg.idProduct) + '\n')
        print('Hexadecimal VendorID=' + hex(cfg.idVendor) + ' & ProductID=' + hex(cfg.idProduct) + '\n\n')

    exit(1)        
    dev = usb.core.find(idVendor=0x04d8, idProduct=0x0042)
    if dev is None:
        raise ValueError('Device not found')

    print("Fat Pants Found!")

    dev.set_configuration()

    # get an endpoint instance
    cfg = dev.get_active_configuration()
    intf = cfg[(0,0)]

    ep = usb.util.find_descriptor(
        intf,
        # match the first OUT endpoint
        custom_match = \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

    assert ep is not None

    # write the data
    ep.write('test')

        