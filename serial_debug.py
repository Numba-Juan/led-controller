import serial
# configure the serial connection
ser = serial.Serial(
    port='/dev/ttyUSB0', # replace with your serial port
    baudrate=115200, # replace with your baud rate
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=None
)
# define the bytes to send
# This is our RGB value at the moment
data = b'\x01\x02\x03'
# send the bytes over the serial connection
ser.write(data)
# close the serial connection
ser.close()
