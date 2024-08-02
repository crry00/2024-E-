from detect import *
from decide import *


def main():
    cap = cv2.VideoCapture(2)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
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

    point1=(-5, -5)
    point2=(-5, -5)
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        if chess.moving(frame):
            if cv2.waitKey(1) & 0xFF == ord('s'):
                player=1
                change = chess.find_change(chess.return_mat(), player)
                print(change)
                if(change[0] is not None):
                    point1 = tuple(map(int, chess.boxes[change[1][0]].get_center()))
                    point2 = tuple(map(int, chess.boxes[change[0][0]].get_center()))
                else:
                    point2=(-5, -5)
                    best_move = get_best_move(chess.return_mat(), player)
                    if best_move is None:
                        print("Game already finished")
                    else:
                        point1 = tuple(map(int, chess.boxes[best_move].get_center()))
                        
                        print(f"The best move for player {player} is: {best_move}")
        cv2.circle(frame, point1, 20, (0, 255, 0), 2)
        cv2.circle(frame, point2, 20, (0, 0, 255), 2)
        cv2.imshow("frame", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
