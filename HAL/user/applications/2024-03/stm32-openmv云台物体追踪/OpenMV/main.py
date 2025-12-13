import sensor, image, time, pyb
from pyb import UART

# red_threshold   = (0, 51, 2, 58, -11, 44)
red_threshold   = (14, 68, 11, 70, 9, 56) #红色阈值设定

sensor.reset() # 初始化摄像头传感器.
sensor.set_pixformat(sensor.RGB565) # 使用RGB565.
sensor.set_framesize(sensor.QVGA) # 使用QVGA.
sensor.skip_frames(10) # 让新设置生效.
sensor.set_auto_whitebal(False) # 关闭自动白平衡.
clock = time.clock() # Tracks FPS.

uart = UART(3, 115200)
def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob.pixels() > max_size:
            max_blob=blob
            max_size = blob.pixels()
    return max_blob

led = pyb.LED(1)
while(True):
    img = sensor.snapshot() # 拍照并返回图像.
    blobs = img.find_blobs([red_threshold])
    x_max = 320
    x_min = 0
    x_1 = 115
    x_2 = 185

    y_max = 240
    y_min = 0
    y_1 = 95
    y_2 = 145

    if blobs:
        max_blob=find_max(blobs)
        print('sum :', len(blobs))
        img.draw_rectangle(max_blob.rect())
        img.draw_cross(max_blob.cx(), max_blob.cy())
        if max_blob.cx()>= x_min  and max_blob.cx() <= 160 and max_blob.cy() >= 120 and max_blob.cy() <= y_max :
            flag = 1
        if max_blob.cx()>=160 and max_blob.cx() <= x_max and max_blob.cy() >=120 and max_blob.cy() <= y_max :
            flag = 2
        if max_blob.cx()>= x_min and max_blob.cx() <= 160 and max_blob.cy() >= y_min and max_blob.cy() <= 120 :
            flag = 3
        if max_blob.cx()>= 160 and max_blob.cx() <= x_max and max_blob.cy() >= y_min and max_blob.cy() <= 120 :
            flag = 4
        if max_blob.cx()>= x_1 and max_blob.cx() <= x_2 and max_blob.cy() >= y_1 and max_blob.cy() < =y_2 :
            flag = 5
        output_str="%d" %flag #方式1
        #output_str=json.dumps([max_blob.cx(),max_blob.cy()]) #方式2
        led.on()
        print('you send:',output_str)
        uart.write(output_str+'\r\n')
    else:
        print('not found!')
        led.off()
