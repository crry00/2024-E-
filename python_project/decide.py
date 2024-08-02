import numpy as np
import math
import random

def LOG(msg):
    print(msg)

class SmartComputerPlayer:
    def __init__(self, letter):
        self.letter = letter

    def get_move(self, game):
        if len(game.available_moves()) == 9:
            square = random.choice(game.available_moves())
        else:
            square = self.minimax(game, self.letter)['position']
        return square

    def minimax(self, state, player):
        max_player = self.letter  # yourself
        other_player = -1 if player == 1 else 1

        # first we want to check if the previous move is a winner
        if state.current_winner == other_player:
            return {'position': None, 'score': 1 * (state.num_empty_squares() + 1) if other_player == max_player else -1 * (
                    state.num_empty_squares() + 1)}
        elif not state.empty_squares():
            return {'position': None, 'score': 0}

        if player == max_player:
            best = {'position': None, 'score': -math.inf}  # each score should maximize
        else:
            best = {'position': None, 'score': math.inf}  # each score should minimize
        for possible_move in state.available_moves():
            state.make_move(possible_move, player)
            sim_score = self.minimax(state, other_player)  # simulate a game after making that move

            # undo move
            state.board[possible_move // 3, possible_move % 3] = 0
            state.current_winner = None
            sim_score['position'] = possible_move  # this represents the move optimal next move

            if player == max_player:  # X is max player
                if sim_score['score'] > best['score']:
                    best = sim_score
            else:
                if sim_score['score'] < best['score']:
                    best = sim_score
        return best

class TicTacToe:
    def __init__(self, board):
        self.board = board
        self.current_winner = self.check_winner()

    def check_winner(self):
        for letter in [1, -1]:
            for row in range(3):
                if np.all(self.board[row, :] == letter):
                    return letter
            for col in range(3):
                if np.all(self.board[:, col] == letter):
                    return letter
            if np.all(np.diagonal(self.board) == letter) or np.all(np.diagonal(np.fliplr(self.board)) == letter):
                return letter
        return None

    def make_move(self, square, letter):
        row, col = divmod(square, 3)
        if self.board[row, col] == 0:
            self.board[row, col] = letter
            if self.winner(square, letter):
                self.current_winner = letter
            return True
        return False

    def winner(self, square, letter):
        row_ind = square // 3
        col_ind = square % 3
        row = self.board[row_ind, :]
        if np.all(row == letter):
            self.current_winner = letter
            return True
        column = self.board[:, col_ind]
        if np.all(column == letter):
            self.current_winner = letter
            return True
        if square % 2 == 0:
            diagonal1 = np.diagonal(self.board)
            if np.all(diagonal1 == letter):
                self.current_winner = letter
                return True
            diagonal2 = np.diagonal(np.fliplr(self.board))
            if np.all(diagonal2 == letter):
                self.current_winner = letter
                return True
        return False

    def empty_squares(self):
        return np.any(self.board == 0)

    def num_empty_squares(self):
        return np.count_nonzero(self.board == 0)

    def available_moves(self):
        return [i * 3 + j for i in range(3) for j in range(3) if self.board[i, j] == 0]

def get_best_move(board, player):
    game = TicTacToe(board)
    if game.current_winner is not None:
        return None  # 游戏已结束，无需再移动
    smart_player = SmartComputerPlayer(letter=player)
    best_move = smart_player.get_move(game)
    return best_move

# 示例用法
if __name__ == '__main__':
    # 已经结束的棋局状态，白棋赢
    board = np.array([[1, -1, 0],
                      [-1, -1, 1], 
                      [1, 1, -1]])
    player = 1  # 白棋
    best_move = get_best_move(board, player)
    
    if best_move is None:
        print("Game already finished")
    else:
        print(f"The best move for player {player} is: {best_move}")
