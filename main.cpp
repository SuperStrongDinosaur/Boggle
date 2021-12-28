#include "boggle.hpp"
#include <fstream>

int main(int argc, const char* argv[]) {
    Results results;
    std::cout << "0 - exit\n1 - load dictionary\n2 - free dictionary\n3 - find words\n4 - free words\n";
    while (true) {
        int command;
        std::cin >> command;

        switch (command) {
            case 0:
                return 0;
            case 1:
                {
                    std::string path;
                    std::cin >> path;
                    clock_t t0 = clock();
                    LoadDictionary(path.c_str());
                    std::cout << "time: " << static_cast<float>(clock() - t0) / CLOCKS_PER_SEC << std::endl;
                }
                break;
            case 2:
                std::cout << "free ditionary" << std::endl;
                FreeDictionary();
                break;
            case 3:
                {
                    std::string board;
                    int w, h;
                    std::cin >> board >> w >> h;
                    clock_t t0 = clock();
                    if (board == "rnd") {
                        board.resize(w * h);
                        srand(t0);
                        std::generate(board.begin(), board.end(), [] { return static_cast<char>(std::rand() % 26 + 'a'); });
                    }
                    results = FindWords(board.c_str(), w, h);
                    std::cout << "time: " << static_cast<float>(clock() - t0) / CLOCKS_PER_SEC << std::endl;
                    for (size_t y = 0; y < h; y++) {
                        for (size_t x = 0; x < w; x++) {
                            std::cout << board[y * w + x];
                        }
                        std::cout << std::endl;
                    }
                    std::cout << "score: " << results.Score << " " << results.Count << std::endl;
                    for (size_t i = 0; i < static_cast<int>(results.Count); i++) {
                        std::cout << results.Words[i] << std::endl;
                    }
                    std::cout << std::endl;
                }
                break;
            case 4:
                std::cout << "free words" << std::endl;
                FreeWords(results);
                break;
            default:
                break;
        }
    }
    return 0;
}
