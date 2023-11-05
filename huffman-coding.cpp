#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <filesystem>  // For handling directories
#include <algorithm>   // For handling different file types
#include <iterator>
#include <string>
#include <cassert>

using namespace std;
namespace fs = std::filesystem;

// Structure to represent a node in the Huffman tree
struct Node {
    char data;
    int frequency;
    Node* left;
    Node* right;

    Node(char d, int f) : data(d), frequency(f), left(nullptr), right(nullptr) {}

    // Function to check if a node is a leaf node
    bool isLeaf() const {
        return (left == nullptr && right == nullptr);
    }
};

// Functor for priority queue comparison
struct Compare {
    bool operator()(Node* left, Node* right) const {
        return left->frequency > right->frequency;
    }
};

// Generate the frequency map for characters in a file
unordered_map<char, int> getFrequencies(const string& filePath) {
    ifstream file(filePath, ios::binary);
    unordered_map<char, int> frequencies;

    if (file.is_open()) {
        char c;
        while (file.get(c)) {
            frequencies[c]++;
        }

        file.close();
    } else {
        cerr << "Error opening file: " << filePath << endl;
    }

    return frequencies;
}

// Build the Huffman tree based on the character frequencies
Node* buildHuffmanTree(const unordered_map<char, int>& frequencies) {
    priority_queue<Node*, vector<Node*>, Compare> pq;

    // Create a leaf node for each character and add it to the priority queue
    for (const auto& pair : frequencies) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Build the Huffman tree by combining the nodes in the priority queue
    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();

        // Create a new parent node with the combined frequency
        Node* parent = new Node('\0', left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;

        pq.push(parent);
    }

    // Return the root of the Huffman tree
    return pq.top();
}

// Generate Huffman codes for each character in the tree
void generateCodes(Node* root, const string& currentCode, unordered_map<char, string>& codes) {
    if (root == nullptr) {
        return;
    }

    // If the node is a leaf node, assign the code
    if (root->isLeaf()) {
        codes[root->data] = currentCode;
    }

    // Recursively generate codes for left and right subtrees
    generateCodes(root->left, currentCode + "0", codes);
    generateCodes(root->right, currentCode + "1", codes);
}

// Encode the input file using Huffman codes
void encodeFile(const string& inputPath, const string& outputPath, const unordered_map<char, string>& codes) {
    ifstream inputFile(inputPath, ios::binary);
    ofstream outputFile(outputPath, ios::binary | ios::trunc);

    if (inputFile.is_open() && outputFile.is_open()) {
        char c;
        bitset<8> bits;
        int bitIndex = 7;

        while (inputFile.get(c)) {
            // Encode each character in the input file using Huffman codes
            const string& code = codes.at(c);
            for (char bit : code) {
                if (bit == '0') {
                    bits[bitIndex] = 0;
                } else if (bit == '1') {
                    bits[bitIndex] = 1;
                }

                if (bitIndex == 0) {
                    // Write the encoded byte to the output file
                    outputFile.put(bits.to_ulong());
                    bits.reset();
                    bitIndex = 7;
                } else {
                    bitIndex--;
                }
            }
        }

        // Write any remaining bits to the output file
        if (bitIndex != 7) {
            outputFile.put(bits.to_ulong());
        }

        inputFile.close();
        outputFile.close();
    } else {
        cerr << "Error opening files: " << inputPath << ", " << outputPath << endl;
    }
}

// Clean up the Huffman tree by deleting all nodes
void deleteTree(Node* root) {
    if (root == nullptr) {
        return;
    }

    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Compress a file by building Huffman tree and encoding it
void compressFile(const string& inputPath, const string& outputPath) {
    unordered_map<char, int> frequencies = getFrequencies(inputPath);
    Node* root = buildHuffmanTree(frequencies);
    unordered_map<char, string> codes;
    generateCodes(root, "", codes);
    encodeFile(inputPath, outputPath, codes);

    // Clean up the Huffman tree
    deleteTree(root);
}

// Compress a directory by encoding each file within it
void compressDirectory(const string& inputDirPath, const string& outputDirPath) {
    if (!fs::exists(inputDirPath) || !fs::is_directory(inputDirPath)) {
        cerr << "Input directory does not exist: " << inputDirPath << endl;
        return;
    }

    if (!fs::exists(outputDirPath)) {
        fs::create_directories(outputDirPath);
    }

    for (const auto& entry : fs::directory_iterator(inputDirPath)) {
        if (fs::is_regular_file(entry)) {
            const string& inputFilePath = entry.path().string();
            const string& outputFilePath = outputDirPath + "/" + entry.path().filename().string() + ".huff";

            compressFile(inputFilePath, outputFilePath);
        }
    }
}

int main() {
    string inputPath = "input.txt";
    string outputPath = "output.huff";

    // Compress a single file
    compressFile(inputPath, outputPath);

    // Compress a directory (multiple files)
    string inputDirPath = "input_directory";
    string outputDirPath = "output_directory";
    compressDirectory(inputDirPath, outputDirPath);

    return 0;
}
