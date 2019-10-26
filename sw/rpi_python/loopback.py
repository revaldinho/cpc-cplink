import RPi.GPIO as gpio
from collections import deque

#define BCM Pin Allocations
PIN_DATA = [11,10,9,8,7,4,3,2] # Data[7:0]
PIN_DIR = 17
PIN_SI = 18
PIN_SOB = 22
PIN_DOR = 23
PIN_WNR = 24

def setup_pins():
    gpio.setmode(gpio.BCM)
    gpio.setup(PIN_DATA,gpio.IN)
    gpio.setup(PIN_DIR, gpio.IN)
    gpio.setup(PIN_DOR, gpio.IN)
    gpio.setup(PIN_SI, gpio.OUT, initial=gpio.LOW)
    gpio.setup(PIN_SOB, gpio.OUT, initial=gpio.LOW)
    gpio.setup(PIN_WNR, gpio.OUT, initial=gpio.LOW)

def write_fifo_byte(txdata):
    gpio.output(PIN_WNR,gpio.HIGH)
    for (b,d) in zip(PIN_DATA,txdata):
        gpio.setup(b, gpio.OUT, initial=d)
    gpio.output(PIN_SI, gpio.HIGH)
    gpio.output(PIN_SI, gpio.LOW)
    for b in PIN_DATA:
        gpio.setup(b, gpio.IN)
    gpio.output(PIN_WNR, gpio.LOW)

def read_fifo_byte():
    rcv = [ gpio.input(b) for b in PIN_DATA ]
    gpio.output(PIN_SOB, gpio.HIGH)
    gpio.output(PIN_SOB, gpio.LOW)
    return(rcv)

if __name__ == "__main__":
    setup_pins()
    write_queue = deque(maxlen=1024)
    while True:
        if gpio.input(PIN_DOR):
            write_queue.append(read_fifo_byte())
        if len(write_queue)>0 and gpio.input(PIN_DIR):
            write_fifo_byte(write_queue.pop())

gpio.cleanup()

