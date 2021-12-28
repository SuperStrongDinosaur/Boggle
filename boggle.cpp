#include "boggle.hpp"
#include <fstream>
#include <thread>

std::unique_ptr<Node> Boggle::dictionary;
std::set<std::string> Boggle::words;

Node* Node::getChild(char c) {
    if (this == nullptr)
        return nullptr;
    return children[c - 'a'];
}

void Node::setChild(char c, Node* node) {
    children[c - 'a'] = node;
}

Node::Node() {
    for (size_t i = 0; i < LettersNum; i++)
        children[i] = nullptr;
}

Node::~Node() {
    for (size_t i = 0; i < LettersNum; i++)
        if (children[i] != nullptr)
            delete children[i];
}

std::pair<int, int> Boggle::getNextCoord() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (q.empty())
        return std::make_pair(-1, -1);
    auto t = q.front();
    q.pop();
    return t;
}

char Boggle::getChar(const unsigned x, const unsigned y) {
    return b[x + y * w];
}

void Boggle::dfsSearch(unsigned x, unsigned y, Node* current, std::vector<std::vector<bool>>& visited, std::set<std::string>& words) {
    if (current == nullptr)
        return;

    visited[x][y] = true;
    if (current->word != "") {
        words.insert(current->word);
    }

    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if ((dx != 0 || dy != 0) && x + dx >= 0 && x + dx < w && y + dy >= 0 && y + dy < h) {
                unsigned newX = x + dx;
                unsigned newY = y + dy;
                if (!visited[newX][newY])
                    dfsSearch(newX, newY, current->getChild(getChar(newX, newY)), visited, words);
            }
    visited[x][y] = false;
}

Boggle::Boggle(const char* board, const unsigned wt, const unsigned ht) : b(board, board + wt * ht), w(wt), h(ht) {
    std::transform(b.begin(), b.end(), b.begin(), tolower);
    for (unsigned i = 0; i < w; i++)
        for (unsigned j = 0; j < h; j++)
            q.push(std::make_pair(i, j));
}

 void Boggle::solver(std::set<std::string>& words) {
    auto vis = std::vector<std::vector<bool>>(w, std::vector<bool>(h));
    std::pair<int, int> cur;
    while ((cur = getNextCoord()) != std::make_pair(-1, -1))
        dfsSearch(cur.first, cur.second, dictionary->getChild(getChar(cur.first, cur.second)), vis, words);
}

void LoadDictionary(const char* path) {
    std::ifstream infile(path);
    std::string word;

    while (std::getline(infile, word)) {
        Boggle::insertToDictionary(word);
    }
}

void FreeDictionary() {
    Boggle::clearDictionary();
}

unsigned inline wordScore(const std::string& word) {
    size_t length = word.length();
    if (length <= 4) return 1;
    else if (length == 5) return 2;
    else if (length == 6) return 3;
    else if (length == 7) return 5;
    else return 11;
}

void solver(Boggle& b, std::set<std::string>& words) {
    b.solver(words);
}

Results FindWords(const char* board, unsigned width, unsigned height) {
    if (width * height < 2 || !std::all_of(board, board + width * height, isalpha)) {
        std::cerr << "wrong board!" << std::endl;
        return Results();
    }
    
    Boggle b(board, width, height);

    const unsigned NUM_THREADS = 4;

    std::vector<std::thread> threads;
    std::vector<std::set<std::string>> sets(NUM_THREADS);

    for (size_t i = 0; i < NUM_THREADS; i++) {
        std::thread thr(solver, std::ref(b), std::ref(sets[i]));
        threads.emplace_back(std::move(thr));
    }

    for (size_t i = 0; i < threads.size(); i++) {
        threads[i].join();
        Boggle::getWords().insert(sets[i].begin(), sets[i].end());
    }

    Results results;
    results.Count = (unsigned)Boggle::getWords().size();
    results.Score = 0;
    if (results.Count > 0) {
        const char** words = new const char* [results.Count];
        size_t i = 0;
        for (const auto& word : Boggle::getWords()) {
            words[i++] = word.c_str();
            results.Score += wordScore(word);
        }

        results.Words = words;

    } else {
        results.Words = nullptr;
    }
    return results;
}

void FreeWords(Results results) {
    delete[] results.Words;
    results.Words = nullptr;
    Boggle::clearWords();
}
