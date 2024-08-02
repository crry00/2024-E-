import cv2
import numpy as np
import math
from info import *



class Box:
    EMPTY = 0
    BLACK = 1
    WHITE = 2

    def __init__(self, center, status=EMPTY):
        self.center = center
        self.status = status

    def get_center(self):
        return self.center

    def get_status(self):
        return self.status

    def set_status(self, status):
        self.status = status

class Chess:
    def __init__(self):
        self.boxes = []
        self.points = []
        self.angle = 0
        self.chessboard_hsv = [(20, 61, 0), (50, 255, 255)]
        self.box_hsv = [(0, 92, 141), (66, 255, 255)]
        self.white_hsv = [(71,0,146), (179, 255, 255)]
        self.black_hsv = [(0, 0, 0), (179, 255, 69)]
        self.hsv = None
        self.last_board=np.zeros((3,3))

    def get_hsv(self, img):
        if img is None:
            LOG("get_hsv():img is empty")
            return False
        self.hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        return True
    def update(self):
        if self.hsv is None:
            LOG("update():hsv is empty")
            return False
        
 

        mask = cv2.inRange(self.hsv, np.array(self.chessboard_hsv[0]), np.array(self.chessboard_hsv[1]))
        #cv2.dilate(mask, mask, None, iterations=2)
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        max_box = None
        max_area = 0
        for contour in contours:
            area = cv2.contourArea(contour)
            if area < 500:
                continue
            rect = cv2.minAreaRect(contour)
            width, height = rect[1]
            ratio = width / height
            if 0.8 < ratio < 1.2 and area > max_area:
                max_box = rect
                max_area = area
        SHOW("hsvmask2", mask)
        
        if max_box is None:
            LOG("update():no chessboard")
            return False

        self.points.clear()
        corners = cv2.boxPoints(max_box)
        lt, rt, rb, lb = corners
        center = max_box[0]
        self.points.append(center)
        self.points.append(tuple(np.array(lt) + (np.array(center) - np.array(lt)) / 2.8))
        self.points.append(tuple(np.array(rt) + (np.array(center) - np.array(rt)) / 2.8))
        self.points.append(tuple(np.array(self.points[1]) + (np.array(self.points[2]) - np.array(self.points[1])) / 2))
        self.points.append(tuple(np.array(rb) + (np.array(center) - np.array(rb)) / 2.8))
        self.points.append(tuple(np.array(lb) + (np.array(center) - np.array(lb)) / 2.8))
        self.points.append(tuple(np.array(self.points[5]) + (np.array(self.points[4]) - np.array(self.points[5])) / 2))
        self.points.append(tuple(np.array(self.points[1]) + (np.array(self.points[5]) - np.array(self.points[1])) / 2))
        self.points.append(tuple(np.array(self.points[2]) + (np.array(self.points[4]) - np.array(self.points[2])) / 2))
        return True


    def find_change(self,now_board,player):
        last_num=np.count_nonzero(self.last_board==0)
        print(last_num)
        now_num=np.count_nonzero(now_board==0)
        print(now_num)
        if  last_num!=now_num:
            self.last_board=now_board
            return [None,None]
        else:
            if(last_num==9):
                return [None,None]
            sub_board=np.subtract(now_board,self.last_board)
            indices_after= np.where(sub_board == player)
            if not indices_after[0].size:
                return [None, None]
            # 将二维索引转换为一维索引
            flat_indices_after = np.ravel_multi_index(indices_after, sub_board.shape)
            indices_before= np.where(sub_board == -player)
            # 将二维索引转换为一维索引
            flat_indices_before = np.ravel_multi_index(indices_before, sub_board.shape)
            return [flat_indices_after, flat_indices_before]


    def reorder(self):
        self.boxes.clear()
        self.points.sort(key=lambda point: point[1])
        up = self.points[:3]
        middle = self.points[3:6]
        down = self.points[6:]

        up.sort(key=lambda point: point[0])
        middle.sort(key=lambda point: point[0])
        down.sort(key=lambda point: point[0])

        self.boxes.extend([Box(center, Box.EMPTY) for center in up])
        self.boxes.extend([Box(center, Box.EMPTY) for center in middle])
        self.boxes.extend([Box(center, Box.EMPTY) for center in down])

    def show_box(self, img):
        if len(self.boxes) != 9:
            LOG("show_box():box size error")
            return
        for i, box in enumerate(self.boxes):
            center = tuple(map(int, box.get_center()))
            cv2.putText(img, str(i), center, cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255, 255, 255), 1)
            color = (255, 0, 0) if box.get_status() == Box.EMPTY else (0, 0, 255) if box.get_status() == Box.BLACK else (255, 255, 0)
            cv2.circle(img, center, 5, color, -1)
        cv2.putText(img, str(self.angle), (100, 50), cv2.FONT_HERSHEY_SIMPLEX, 2, (255, 0, 0), 1)


 
    def track(self):
        if not self.boxes or not self.points:
            LOG("track():box or points empty")
            return 0

        new_boxes = []
        for box in self.boxes:
            center = np.array(box.get_center())
            self.points.sort(key=lambda p: np.linalg.norm(np.array(p) - center))
            if np.linalg.norm(np.array(self.points[0]) - center) > 50:
                return 0
            else:
                new_boxes.append(Box(self.points[0], box.get_status()))

        for nowpoint in new_boxes:
            flagcount = 0
            for point in new_boxes:
                dis = np.linalg.norm(np.array(point.get_center()) - np.array(nowpoint.get_center()))
                if dis < 10.0:
                    flagcount += 1
                if flagcount >= 2:
                    LOG("find")
                    return -1

        self.boxes = new_boxes
        return 1


    def get_angle(self):
            if not self.boxes:
                LOG("get_angle():box empty")
                return
            point1 = np.array(self.boxes[1].get_center())
            point2 = np.array(self.boxes[7].get_center())
            x = abs(point1[0] - point2[0])
            y = abs(point1[1] - point2[1])
            LOG(x)
            LOG(y)
            if x < 1 or y < 1:
                self.angle = 0
                return
            self.angle = abs(math.atan2(x, y) * 180 / math.pi)
            if point1[0] > point2[0]:
                self.angle = -self.angle

    def get_chess_color(self):
        if not self.boxes:
            LOG("get_chess_color():box empty")
            return
        if self.hsv is None:
            LOG("get_chess_color():hsv empty")
            return

        mask = np.zeros(self.hsv.shape[:2], dtype=np.uint8)
        r = 20
        for box in self.boxes:
            cv2.circle(mask, tuple(map(int, box.get_center())), r, 255, -1)
            box.set_status(Box.EMPTY)

        white_mask = cv2.inRange(self.hsv, np.array(self.white_hsv[0]), np.array(self.white_hsv[1]))
        black_mask = cv2.inRange(self.hsv, np.array(self.black_hsv[0]), np.array(self.black_hsv[1]))
        #cv2.dilate(white_mask, white_mask, None, iterations=1)
        #cv2.dilate(black_mask, black_mask, None, iterations=1)

        white_mask = cv2.bitwise_and(white_mask, mask)
        black_mask = cv2.bitwise_and(black_mask, mask)

        contours, _ = cv2.findContours(white_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        white_points = [cv2.minAreaRect(contour)[0] for contour in contours if cv2.contourArea(contour) >= 50]
        contours, _ = cv2.findContours(black_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        black_points = [cv2.minAreaRect(contour)[0] for contour in contours if cv2.contourArea(contour) >= 10]

        for box in self.boxes:
            box_center = np.array(box.get_center())
            white_points.sort(key=lambda p: np.linalg.norm(np.array(p) - box_center))
            if white_points and np.linalg.norm(np.array(white_points[0]) - box_center) <= r:
                box.set_status(Box.WHITE)
                LOG("white")
            black_points.sort(key=lambda p: np.linalg.norm(np.array(p) - box_center))
            if black_points and np.linalg.norm(np.array(black_points[0]) - box_center) <= r:
                box.set_status(Box.BLACK)
                LOG("black")

        SHOW("mask", mask)
        SHOW("whilt_mask", white_mask)
        SHOW("black_mask", black_mask)

 
    def normal(self, img):
        if img is None:
            LOG("normal():image is empty")
            return False
        if not self.get_hsv(img):
            LOG("normal():get hsv failed")
            return False
        if self.update():
            self.reorder()
        self.get_chess_color()
        self.show_box(img)
        return True


    def moving(self, img):
        if img is None:
            LOG("moving():image is empty")
            return False
        if not self.get_hsv(img):
            LOG("moving():get hsv failed")
            return False
        if self.update():
            if self.track() == -1:
                LOG("track() lost!")
                self.reorder()
            self.get_angle()
        self.get_chess_color()
        self.show_box(img)
        return True

    def return_mat(self):
        chess_mat = np.zeros((3, 3), dtype=np.int8)
        if len(self.boxes) != 9:
            LOG("return_mat():box is empty")
            return chess_mat
        for i, box in enumerate(self.boxes):
            if box.get_status() == Box.WHITE:
                chess_mat[i // 3, i % 3] = 1
            elif box.get_status() == Box.BLACK:
                chess_mat[i // 3, i % 3] = -1
        LOG(chess_mat)
        return chess_mat

    def init(self, img):
        if img is None:
            LOG("init():image is empty")
            return False
        if not self.get_hsv(img):
            LOG("init():get hsv failed")
            return False
        if not self.update():
            LOG("init():update failed")
            return False
        self.reorder()
        self.show_box(img)
        return True
    
def main():
    cap = cv2.VideoCapture(2)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 360)
    if not cap.isOpened():
        return
    chess = Chess()


    while True:
        ret, frame = cap.read()
        if not ret:
            break
        if chess.init(frame):
            LOG("init")
            break


    while True:
        ret, frame = cap.read()
        if not ret:
            break
        if chess.moving(frame):
            chess.return_mat()
        cv2.imshow("frame", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
