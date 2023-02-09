import serial
ser = serial.Serial('COM8', 115200, timeout=None)
with open('data.txt', 'w') as f:
    print("start:")
    f.write("t, T, P, I, D, C \n")

try:
    while True:
        data = ser.readline().decode()
        if data.strip():
            print(data)
            with open('data.txt', 'a+') as f:
                f.write(ser.readline().decode())
except KeyboardInterrupt:
    pass
