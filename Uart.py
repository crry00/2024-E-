import time
import serial


class Uart:
    def __init__(self, com="/dev/ttyUSB0"):
        # 实例化串口对象
        self.uart = serial.Serial(com, 115200, timeout=0.01)
        if self.uart.isOpen():
            print("uart is ready")

    def close(self):
        self.uart.close()

    # def car_ready(self, wait=True):
    #     print(f"car is ready")
    #     self.uart_send_command(task_id=99, param1=0, param2=0, wait=wait, timeout=99999)

    def uart_send_command(self, task_id, param1, param2, wait=True, timeout=10):
        print(f"@{task_id:02d}!{param1:+04d}|{param2:+04d}#")
        self.uart.write(f"@{task_id:02d}!{param1:+04d}|{param2:+04d}#".encode("utf-8"))
        time.sleep(0.1)  # 避免单片机清零不及时
        if wait is True:
            self.wait_for_32_ack(timeout, task_id)

    def wait_for_32_ack(self, timeout=10, *args, **kwargs):
        time_begin = time.time()
        time_end = time_begin + timeout
        task = list(args)
        while task:
            if time.time() >= time_end:
                print("time out")
                break
            if self.uart.inWaiting() >= 2:
                rx_buf = self.uart.read(2).decode("utf-8")
                ack = int(rx_buf)
                if ack in task:
                    print(f"get  {ack}")
                    task.remove(ack)

            else:
                time.sleep(0.02)
        print(f"get ack {args}")

    def car_move_xy_cm(self, x, y, wait=True):
        print(f"car X{x} Y{y}")
        self.uart_send_command(task_id=1, param1=x, param2=y, wait=wait)

    def car_move_xy_mm(self, x, y, wait=True):
        print(f"car X{x} Y{y}")
        self.uart_send_command(task_id=9, param1=x, param2=y, wait=wait)

    def car_move_calibration(self, wait=True):
        print(f"car_move_calibration")
        self.uart_send_command(task_id=7, param1=0, param2=0, wait=wait)

    def car_move_interval(self, length, wait=True):
        print(f"car length{length}")
        self.car_move_xy_mm(0, 150 * length, wait=wait)

    def car_move_angle(self, angle, wait=True):
        print(f"car angle{angle}")
        self.uart_send_command(task_id=2, param1=angle, param2=0, wait=wait)

    #  高位0 低位1
    #  内位0 外位1

    def arm_move_height(self, height, wait=True):
        print(f"arm height{height}")
        height = 0 if height == "high" else 1
        self.uart_send_command(task_id=3, param1=height, param2=0, wait=wait)

    def arm_move_angle(self, angle, wait=True):
        print(f"car angle{angle}")
        angle = 1 if angle == "out" else 0
        self.uart_send_command(task_id=4, param1=angle, param2=0, wait=wait)

    def arm_upload(self, disc_id, height, wait=True):
        print(f"arm upload{disc_id}")
        height = 1 if height == "high" else 0
        self.uart_send_command(task_id=5, param1=disc_id, param2=height, wait=wait)

    def arm_download(self, disc_id, height, wait=True):
        print(f"arm download{height}")
        height = 1 if height == "high" else 0

        self.uart_send_command(task_id=6, param1=disc_id, param2=height, wait=wait)

    def send_lcd_tasks(self, task_list0, task_list1, wait=True):

        task0 = task_list0[0] * 100 + task_list0[1] * 10 + task_list0[2]
        task1 = task_list1[0] * 100 + task_list1[1] * 10 + task_list1[2]
        print(f"arm send_lcd_tasks{task0}+{task1}")
        self.uart_send_command(task_id=8, param1=task0, param2=task1, wait=wait)

#
