from machine import Pin, SoftI2C
import ssd1306
i2c = SoftI2C(Pin(18, Pin.OUT, Pin.PULL_UP), Pin(5, Pin.OUT, Pin.PULL_UP), freq=80000)
print(i2c.scan())
display = ssd1306.SSD1306_I2C(128, 64, i2c)
# display.text('Hello, World!', 0, 0, 1)
display.fill(1)
from uQR import QRCode
qr = QRCode()
url = "https://hisa.dev/?hogehogehogehogehttps"
qr.add_data(url)
matrix = qr.get_matrix()
ratio = 2
dot_size = 2
print(["URL", len(url), "QR",len(matrix)])
w = 128
h = 64
for dot_y in range(len(matrix)):
    for dot_x in range(len(matrix[0])):
        x = int(w/2  + ( dot_x - len(matrix[0])/2) * dot_size)
        y = int(h/2  + ( dot_y - len(matrix)/2) * dot_size)
        value = not matrix[dot_y][dot_x]
        display.fill_rect(x, y, x+dot_size, y+dot_size, value)
display.show()