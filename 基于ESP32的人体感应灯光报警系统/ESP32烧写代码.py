from machine import UART,Pin
from umqtt.simple import MQTTClient
import _thread
import network
import time
global wlan
global c
# 初始化一个UART对象
uart = UART(2, baudrate=115200, rx=13,tx=12,timeout=10)
feng=Pin(25,Pin.OUT)        
ren=Pin(26,Pin.IN)
led=Pin(2,Pin.OUT)

wlan=network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect('huangdaqian_plus','87468792')
while wlan.isconnected() == False:        #循环来等待连上wifi
    time.sleep(2)
print('Connected to wifi successfully!')
c=MQTTClient('hang1','121.37.14.128',port=1883,user='',password='')
c.connect()




def write():
  while True:
    if ren.value()==1:
      text="unsafe!"
      test_ren="1"
      test_feng="1"
      c.publish('hang','Warning!')
      print("有人")
      feng.value(0)
    elif ren.value()==0:
      text="safe!"
      test_ren="0"
      test_feng="0"
      c.publish('hang','Safe!')
      print("没人")
      feng.value(1)
    if led.value()==1:
      test_led="1"
    elif led.value()==0:
      test_led="0"
    # 发送一条消息
    print('Send Byte :') # 发送字节数
    uart.write('{0},{1},{2},{3}'.format(text,test_led,test_feng,test_ren))
    # 等待1s
    time.sleep(3)
  
def read():
  while True:
    if uart.any():
        # 如果有数据 读入一行数据返回数据为字节类型
        bin_data = uart.readline()
        if bin_data=="b'1'":
          led.value(1)
        elif bin_data=="b'0'":
          led.value(0)
        # 将手到的信息打印在终端
        print('Echo Byte: {}'.format(bin_data))
        # 将字节数据转换为字符串 字节默认为UTF-8编码
        print('Echo String: {}'.format(bin_data.decode()))
   
_thread.start_new_thread(write,())
_thread.start_new_thread(read,())


