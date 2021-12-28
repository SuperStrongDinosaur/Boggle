#ifndef boggle_hpp
#define boggle_hpp

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
#include <mutex>

struct Results {
    const char* const* Words;    // pointers to unique found words, each terminated by a non-alpha char
    unsigned           Count;    // number of words found
    unsigned           Score;    // total score
    void* UserData; // ignored by test framework; can use for your own purposes
};

// input dictionary is a file with one word per line
void LoadDictionary(const char* path);
void FreeDictionary();
// `board` is row-major and exactly `width` * `height` chars; char 'q' represents the 'qu' Boggle cube
Results FindWords(const char* board, unsigned width, unsigned height);
// `results` is identical to what was returned from `FindWords`
void FreeWords(Results results);

class Node {
private:
    static constexpr unsigned LettersNum = 'z' - 'a' + 1;
    Node* children[LettersNum];

public:
    std::string word;

    Node* getChild(char c);
    void setChild(char c, Node* node);
    Node();
    ~Node();

    Node(const Node &) = delete;
    Node& operator=(const Node &) = delete;
};

class Boggle {
private:
    std::vector<char> b;
    std::queue<std::pair<int, int>> q;

    std::mutex queueMutex;

    static std::unique_ptr<Node> dictionary;
    static std::set<std::string> words;

    unsigned w;
    unsigned h;

    std::pair<int, int> getNextCoord();
    char getChar(const unsigned x, const unsigned y);
    void dfsSearch(unsigned x, unsigned y, Node* current, std::vector<std::vector<bool>>& visited, std::set<std::string>& words);
    
    bool static inline validateWord(std::string& word) {
        word.erase(std::remove(word.begin(), word.end(), '\n'), word.end());
        word.erase(std::remove(word.begin(), word.end(), '\r'), word.end());
        if (word.length() < 3 || !std::all_of(word.begin(), word.end(), isalpha))
            return false;
        for (size_t i = word.find('q'); i != std::string::npos; i = word.find('q', i + 1))
            if (i == word.length() - 1 || word[i + 1] != 'u')
                return false;
        return true;
    }

public:
    Boggle(const char* board, const unsigned wt, const unsigned ht);
    void solver(std::set<std::string>& words);
    
    static std::set<std::string>& getWords()  {
        return words;
    }
    
    static void clearDictionary() {
        dictionary.reset(nullptr);
    }
    
    static void clearWords()  {
        words.clear();
    }
    
    static void insertToDictionary(std::string word)  {
        if (validateWord(word)) {
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);

            if (dictionary == nullptr) {
                dictionary.reset(new Node());
            }
            auto p = dictionary.get();

            for (size_t i = 0; i < word.length(); i++) {
                if (!p->getChild(word[i]))
                    p->setChild(word[i], new Node());

                p = p->getChild(word[i]);
                if ((word[i] == 'q') && (word[i + 1] == 'u'))
                    i++;
            }
            p->word = word;
        }
    }
};

#endif /* boggle_hpp */
