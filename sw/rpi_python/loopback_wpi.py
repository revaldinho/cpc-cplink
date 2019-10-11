import wiringpi as gpio
from collections import deque

#define BCM Pin Allocations
PIN_DATA = [11,10,9,8,7,4,3,2] # Data[7:0]
PIN_SI   = 18
PIN_DIR  = 17
PIN_SOB  = 22
PIN_DOR  = 23
PIN_WNR  = 24
GPIO_IN  = 0 # Pin direction input
GPIO_OUT = 1 # Pin direction output

def setup_pins():
    gpio.wiringPiSetupGpio()
    for b in PIN_DATA:
        gpio.pinMode(b,GPIO_IN)
    gpio.pinMode(PIN_DIR, GPIO_IN)
    gpio.pinMode(PIN_DOR, GPIO_IN)
    gpio.pinMode(PIN_SI, GPIO_OUT)
    gpio.digitalWrite(PIN_SI,0)
    gpio.pinMode(PIN_SOB, GPIO_OUT)
    gpio.digitalWrite(PIN_SOB,1)
    gpio.pinMode(PIN_WNR, GPIO_OUT)
    gpio.digitalWrite(PIN_WNR,0)

def write_fifo_byte(txdata):
    gpio.digitalWrite(PIN_WNR,1)
    for (b,d) in zip(PIN_DATA,txdata):
        gpio.pinMode(b, GPIO_OUT)
        gpio.digitalWrite(b,d)
    gpio.digitalWrite(PIN_SI, 1)
    gpio.digitalWrite(PIN_SI, 0)
    for b in PIN_DATA:
        gpio.pinMode(b, GPIO_IN)
    gpio.digitalWrite(PIN_WNR, 0)

def read_fifo_byte():
    rcv = [ gpio.digitalRead(b) for b in PIN_DATA]
    gpio.digitalWrite(PIN_SOB, 0)
    gpio.digitalWrite(PIN_SOB, 1)
    return(rcv)

if __name__ == "__main__":
    setup_pins()
    write_queue = deque(maxlen=1024)
    while True:
        if gpio.digitalRead(PIN_DOR):
            write_queue.append(read_fifo_byte())
        if len(write_queue)>0 and gpio.digitalRead(PIN_DIR):
           write_fifo_byte(write_queue.pop())
