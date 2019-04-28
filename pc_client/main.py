#!/usr/bin/python3
import usb.core
import usb.util
import sys

def fat_write(message):
    """ Formats stdout print """
    print("==================== {:s} ====================\n".format(message))

def write_to_screen():
    fat_write("Write to the Screen")
    pass

def toggle_light():
    fat_write("Toggling Light on AN0")
    ep.write('\x80')

if __name__ == '__main__':
    fat_write("Welcome to Fat Pants")
    dev = usb.core.find(idVendor=0x04d8, idProduct=0x0042)
    if dev is None:
        raise ValueError('Device not found') 

    fat_write("Fat Pants Found!")
    #config
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

    commands = {1:write_to_screen,2:toggle_light}
    while 1:
       fat_write("Enter '1' to write Enter '2' to toggle light")
       user_input = int(input())
       commands[user_input]()
