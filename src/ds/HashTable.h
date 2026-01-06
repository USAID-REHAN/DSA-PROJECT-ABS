#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <iostream>

using namespace std;

template<typename T>
struct HashNode {
    string key;
    T* value;  // Pointer to actual object
    HashNode* next;

    HashNode(string k, T* v) {
        key = k;
        value = v;
        next = nullptr;
    }
};

template<typename T>
class HashTable {
private:
    static const int SIZE = 50;  // Good size for your 50-100 records
    HashNode<T>* table[SIZE];

    int hashFunc(string key) {
        int hash = 0;
        for (int i = 0; i < key.length(); i++) {
            hash = (hash * 31 + key[i]) % SIZE;
        }
        return hash;
    }

public:
    HashTable() {
        for (int i = 0; i < SIZE; i++) {
            table[i] = nullptr;
        }
    }

    // Insert using chaining
    void insert(string key, T* value) {
        int index = hashFunc(key);
        HashNode<T>* newNode = new HashNode<T>(key, value);

        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            // Chaining for collision resolution
            HashNode<T>* temp = table[index];
            while (temp->next != nullptr) {
                if (temp->key == key) {
                    temp->value = value;  // Update if exists
                    delete newNode;
                    return;
                }
                temp = temp->next;
            }
            if (temp->key == key) {
                temp->value = value;
                delete newNode;
            } else {
                temp->next = newNode;
            }
        }
    }

    // Search by key
    T* search(string key) {
        int index = hashFunc(key);
        HashNode<T>* temp = table[index];

        while (temp != nullptr) {
            if (temp->key == key) {
                return temp->value;
            }
            temp = temp->next;
        }
        return nullptr;
    }

    // Remove by key
    void remove(string key) {
        int index = hashFunc(key);
        HashNode<T>* temp = table[index];
        HashNode<T>* prev = nullptr;

        while (temp != nullptr && temp->key != key) {
            prev = temp;
            temp = temp->next;
        }

        if (temp == nullptr) return;

        if (prev == nullptr) {
            table[index] = temp->next;
        } else {
            prev->next = temp->next;
        }
        delete temp;
    }

    ~HashTable() {
        for (int i = 0; i < SIZE; i++) {
            HashNode<T>* temp = table[i];
            while (temp != nullptr) {
                HashNode<T>* next = temp->next;
                delete temp;
                temp = next;
            }
        }
    }
};

#endif