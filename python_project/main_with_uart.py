from detect import *
from decide import *
from myuart import *

def task3(chess, cap):
    pass
    # while True:
    #     ret, frame = cap.read()
    #     if not ret:
    #         break
    #     chess.init(frame)
    #     flag=True
    #     while flag:
    #         ret, frame = cap.read()
    #         if not ret:
    #             break
    #         flag=chess.normal(frame)
def task4(chess,cap,uart):
    point1=(-5, -5)
    point2=(-5, -5)
    player=-1
    flag=False
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        dates=uart.wait_for_data_packet()
        if dates[0]or dates[1]==52:
            flag=True
        if chess.normal(frame):
            if flag:
                change = chess.find_change(chess.return_mat(), player)
                LOG(change)
                if(change[0] is not None):
                    point1 = tuple(map(int, chess.boxes[change[1][0]].get_center()))
                    point2 = tuple(map(int, chess.boxes[change[0][0]].get_center()))
                    LOG("task4(): change ")
                    
                    uart.task5_change(change[0][0], change[1][0])
                    flag=False
                    
                else:
                    point2=(-5, -5)
                    best_move = get_best_move(chess.return_mat(), player)
                    if best_move is None:
                        LOG("task4(): Game already finished")
                        return
                    else:
                        point1 = tuple(map(int, chess.boxes[best_move].get_center()))
                        
                        uart.task5_put(best_move)
                        flag=False
                    
                    LOG(f"task4() :The best move for player {player} is: {best_move}")
        cv2.circle(frame, point1, 20, (0, 255, 0), 2)
        cv2.circle(frame, point2, 20, (0, 0, 255), 2)

        cv2.imshow("frame", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

def task5(chess,cap,uart):
    point1=(-5, -5)
    point2=(-5, -5)
    player=1
    flag=False
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        dates=uart.wait_for_data_packet()
        if dates[0]or dates[1] ==53:
            flag=True
        if chess.normal(frame):
            if flag:
                change = chess.find_change(chess.return_mat(), player)
                LOG(change)
                if(change[0] is not None):
                    point1 = tuple(map(int, chess.boxes[change[1][0]].get_center()))
                    point2 = tuple(map(int, chess.boxes[change[0][0]].get_center()))
                    LOG("task5(): change ")
                    
                    uart.task5_change(change[0][0], change[1][0])
                    flag=False
                    
                else:
                    point2=(-5, -5)
                    best_move = get_best_move(chess.return_mat(), player)
                    if best_move is None:
                        LOG("task5(): Game already finished")
                        return
                    else:
                        point1 = tuple(map(int, chess.boxes[best_move].get_center()))
                        
                        uart.task5_put(best_move)
                        flag=False
                    
                    LOG(f"task5() :The best move for player {player} is: {best_move}")
        cv2.circle(frame, point1, 20, (0, 255, 0), 2)
        cv2.circle(frame, point2, 20, (0, 0, 255), 2)

        cv2.imshow("frame", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

def main():
    cap = cv2.VideoCapture(2)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    if not cap.isOpened():
        return
    chess = Chess()
    uart=Uart()

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
        chess.normal(frame)

        datas=uart.wait_for_data_packet()
        if datas[0] ==64:
            LOG(f"main data:{datas}")
            if datas[1] == 51:
                pass
            if datas[1] == 52:
                task4(chess,cap,uart)
            if datas[1] == 53:
                task5(chess,cap,uart)
        if datas[0] =='!':
            LOG("END")
            break
        cv2.imshow("frame",frame)
        cv2.waitKey(1)
    
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
