all: game

game: main.cpp snake.cpp max7219.cpp snake_game.cpp
	g++ -o game main.cpp snake.cpp max7219.cpp snake_game.cpp -std=c++17

clean:
	rm -f game

