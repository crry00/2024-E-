import time
import serial
from info import *

class Uart:
    def __init__(self, com="/dev/ttyUSB0"):
        # 实例化串口对象
        self.uart = serial.Serial(com, 9600, timeout=0.01)
        if self.uart.isOpen():
            LOG("uart is ready")

    def close(self):
        self.uart.close()

    def uart_send_command(self, task_id, param1, param2):
        # 将每个参数格式化为单个字符的字符串形式
        task_id_str = f"{task_id:01d}"
        param1_str = f"{param1:01d}" if isinstance(param1, int) else param1
        param2_str = f"{param2:01d}" if isinstance(param2, int) else param2
        
        command_str = f"{task_id_str}{param1_str}{param2_str}"
        LOG(command_str)  # 打印格式化字符串以供调试
        self.uart.write(command_str.encode("utf-8"))  # 写入串口
        time.sleep(0.1)  # 避免单片机清零不及时

    def wait_for_data_packet(self):
        
        first = None
        second = None
        
        if self.uart.inWaiting() >= 2:
            packet = self.uart.read(2)
            LOG(f"packet: {packet}")
            
            first = packet[0]  # 提取task数据
            second = packet[1]  # 提取color数据

                  # 数据已接收，退出循环
        
        if first is None and second is None:
            LOG("Timeout waiting for data packet.")
            return [None, None]  # 超时返回None, None
        else:
            return [first, second]  # 数据包返回

    def task3_move_xy(self, x, y):
        LOG(f"task3: X{x} Y{y}")
        if x < 0:
            x = 150 + x
        if y < 0:
            y = 150 + y
        self.uart_send_command(task_id=3, param1=x, param2=y)

    def task3_move_ok(self):
        LOG("task3: ok")
        self.uart_send_command(task_id=3, param1='@', param2='@')

    def task4_put(self, num):
        num += 1
        LOG(f"task4: put {num}")
        self.uart_send_command(task_id=4, param1='@', param2=num)

    def task4_change(self, num1, num2):
        num1 += 1
        num2 += 1
        LOG(f"task4: return {num1} to {num2}")
        self.uart_send_command(task_id=4, param1=num1, param2=num2)

    def task5_put(self, num):
        num += 1
        LOG(f"task5: put {num}")
        self.uart_send_command(task_id=5, param1='@', param2=num)

    def task5_change(self, num1, num2):
        num1 += 1
        num2 += 1
        LOG(f"task5: return {num1} to {num2}")
        self.uart_send_command(task_id=5, param1=num1, param2=num2)

    def task3_move_angle(self, angle):
        LOG(f"task3 angle {angle}")
        self.uart_send_command(task_id=3, param1='!', param2=angle)



if __name__ == "__main__":
    uart = Uart() 
    uart.wait_for_data_packet()
    uart.task4_change(1, 2)
    # uart.task3_move_xy(0, 150)
    # time.sleep(2)
    # uart.task3_move_angle(90)
    # time.sleep(2)
    # uart.task3_move_angle(-90)
    # time.sleep(2)
    # uart.task4_put(1)
    # time.sleep(2)
    # uart.task4_change